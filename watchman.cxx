
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/signalfd.h>

#include "watchman.hxx"
#include "compiler.hxx"
#include "error.hxx"
#include "child.hxx"
#include "buffer.hxx"
#include "file.hxx"

Watchman::Watchman(Allocator *alloc)
: _alloc(alloc), _sfd(-1), _exit_phase(0)
{
	int i;

	for (i = 0; i < WATCHMAN_MAX_CHILDREN; ++i) {
		_children[i].child  = NULL;
		_children[i].flags  = 0;
		_children[i].buffer = NULL;
		_children[i].fo     = NULL;
		_children[i].fe     = NULL;
	}
}

Watchman::~Watchman()
{
	/* Check that everything is nicely cleaned-up
	 */
}

Allocator *Watchman::alloc()
{
	return _alloc;
}

int Watchman::init_signal_handling()
{
	sigset_t all;
	int err;

	sigfillset(&all);
	err = sigprocmask(SIG_BLOCK, &all, &_default_signal_set);
	if (unlikely(err)) {
		WATCHMAN_ERROR("Failed to block signals: %s", errno);
		return -errno;
	}

	_sfd = signalfd(-1, &all, 0);
	if (unlikely(-1 == _sfd)) {
		WATCHMAN_ERROR("Failed to open signalfd: %s", errno);
		return -errno;
	}

	return 0;
}

int Watchman::fini_signal_handling()
{
	int err;

	err = close(_sfd);
	if (unlikely(err)) {
		WATCHMAN_ERROR("close() failed with errno %d: %s", errno, strerror(errno));
	}

	err = sigprocmask(SIG_BLOCK, &_default_signal_set, NULL);
	if (unlikely(err)) {
		WATCHMAN_ERROR("Failed to reset default signal set");
		return -errno;
	}

	return 0;
}

int Watchman::execute_children()
{
	int i, err, fails;

	if (unlikely(0 == _num_children_left())) {
		WATCHMAN_ERROR("No children in list");
		return -1;
	}

	fails = 0;
	for (i = 0; i < WATCHMAN_MAX_CHILDREN; ++i) {
		if (NULL == _children[i].child)
			continue;

		err = _children[i].child->execute();
		if (unlikely(err)) {
			WATCHMAN_ERROR("Failed to execute() child number %d", i);
			++fails;
		}

		_children[i].flags = WATCHMAN_CHILD_ALIVE;
	}

	if (unlikely(fails == _num_children_left()))
		return -1;

	/* Continue as long as at least one children executed succesfully.
	 */
	return 0;
}

int Watchman::loop()
{
	int err;
	int fails;

	fails = 0;

	_exit_phase = WATCHMAN_EXIT_PHASE_CONT;

	while (WATCHMAN_EXIT_PHASE_QUIT != _exit_phase) {
		err = _poll();
		if (unlikely(err)) {
			++fails;
			if (fails > WATCHMAN_MAX_POLL_FAILS) {
				WATCHMAN_ERROR("Maximal number of poll() failures reached.");
				return -1;
			}
		} else
			fails = 0;

		if (_pfds[0].revents & POLLIN)
			_recv_and_handle_signal();

		_handle_children();

		if (0 == _num_children_left())
			_exit_phase = WATCHMAN_EXIT_PHASE_QUIT;
	}

	return -1;
}

void Watchman::_fill_pollfds()
{
	int i, j;

	memset(_pfds, 0, sizeof(_pfds));

	_pfds[0].fd     = _sfd;
	_pfds[0].events = POLLIN;

	for (i = 0, j = 0; i < WATCHMAN_MAX_CHILDREN; ++i) {
		if (NULL == _children[i].child)
			continue;

		_fill_single_pollfd(&_pfds[1 + 4*j], _children[i].child->file_o(), POLLIN);
		_fill_single_pollfd(&_pfds[2 + 4*j], _children[i].child->file_e(), POLLIN);

		if (_children[i].buffer->stdout_pending()) {
			_fill_single_pollfd(&_pfds[3 + 4*j], _children[i].fo, POLLOUT);
		}

		if (_children[i].buffer->stderr_pending()) {
			_fill_single_pollfd(&_pfds[4 + 4*j], _children[i].fe, POLLOUT);
		}

		++j;
	}

	_num_pfds = 1 + 4*_num_children_left();
}

void Watchman::_fill_single_pollfd(struct pollfd *pfd, File *f, int events)
{
	if ((WATCHMAN_FILE_STATE_STALE == f->state()) && f->can_reopen()) {
		f->reopen();	/* Ignore error. */
	}

	if (unlikely(f->fileno() < 0)) {
		return;
	}

	pfd->fd     = f->fileno();
	pfd->events = events;

}

int Watchman::_poll()
{
	int err;

	_fill_pollfds();

	err = poll(_pfds, _num_pfds, WATCHMAN_POLL_TIMEOUT);
	if (unlikely(err < 0)) {
		WATCHMAN_ERROR("poll failed: %s", errno);
		return -errno;
	}

	return 0;
}

int Watchman::_recv_and_handle_signal()
{
	struct signalfd_siginfo info;
	int err;

	err = read(_sfd, &info, sizeof(info));
	if (unlikely(err < 0)) {
		WATCHMAN_ERROR("read() failed with errno %d: %s", errno, strerror(errno));
		return -errno;
	}
	if (err != sizeof(info)) {
		WATCHMAN_ERROR("read() returned %d (sizeof signalfd_siginfo is %d)\n", err, sizeof(info));
		return -1;
	}

	WATCHMAN_DEBUG("Received signal %d", info.ssi_signo);

	if (SIGALRM == info.ssi_signo)
		return _handle_sigalrm();
	if (SIGCHLD == info.ssi_signo)
		return _handle_sigchld(info.ssi_pid);
	if ((SIGINT  == info.ssi_signo) ||
	    (SIGQUIT == info.ssi_signo) ||
	    (SIGTERM == info.ssi_signo))
		return _handle_sigquit(info.ssi_signo);

	WATCHMAN_WARN("Ignoring signal %d from pid %d (uid %d)", info.ssi_signo, info.ssi_pid, info.ssi_uid);

	return 0;
}

int Watchman::_num_children_left() const
{
	int i, n;

	n = 0;
	for (i = 0; i < WATCHMAN_MAX_CHILDREN; ++i)
		n += (NULL != _children[i].child);

	return n;
}

int Watchman::_find_child_by_pid(long long pid) const
{
	int i, j;

	for (i = 0, j = -1; i < WATCHMAN_MAX_CHILDREN; ++i)
		if (_children[i].child && (pid == _children[i].child->pid())) {
			j = i;
			break;
		}

	return j;
}

int Watchman::_handle_sigchld(long long pid)
{
	int i;

	i = _find_child_by_pid(pid);
	if (unlikely(i < 0)) {
		WATCHMAN_ERROR("Received SIGCHLD but could not find matching children with pid %lld\n", pid);
		return -1;
	}

	/* Keep the child structure alive so that remaining data in the buffers can
	 * be flushed.
	 */
	_children[i].flags = WATCHMAN_CHILD_FINISHED;

	return 0;
}

int Watchman::_handle_sigquit(int signo)
{
	int i;

	if (WATCHMAN_EXIT_PHASE_CONT != _exit_phase) {
		WATCHMAN_WARN("Dropping signal %d while shutting down.", signo);
		return 0;
	}

	for (i = 0; i < WATCHMAN_MAX_CHILDREN; ++i)
		if (_children[i].child && (WATCHMAN_CHILD_ALIVE == _children[i].flags)) {
			WATCHMAN_LOG("Terminating child process %d", _children[i].child->pid());
			_children[i].child->terminate();
		}

	/* TODO Use a timerfd instead.
	 */
	alarm(WATCHMAN_ALARM_SECS);
	_exit_phase = WATCHMAN_EXIT_PHASE_BEGIN;

	return 0;
}

int Watchman::_handle_sigalrm()
{
	int i;

	switch (_exit_phase) {
	case WATCHMAN_EXIT_PHASE_BEGIN:
		/* Received the alarm while termination is in progress. Children should have
		 * been terminated but are not yet apparently. Try to kill them hard.
		 */
		for (i = 0; i < WATCHMAN_MAX_CHILDREN; ++i) {
			if (_children[i].child && (WATCHMAN_CHILD_ALIVE == _children[i].flags)) {
				WATCHMAN_LOG("Killing process %d", _children[i].child->pid());
				_children[i].child->kill();
			}
		}

		alarm(WATCHMAN_ALARM_SECS);
		_exit_phase = WATCHMAN_EXIT_PHASE_AGAIN;
		break;
	case WATCHMAN_EXIT_PHASE_AGAIN:
		_exit_phase = WATCHMAN_EXIT_PHASE_QUIT;
		break;
	default:
		WATCHMAN_ERROR("Invalid exit phase value %d", _exit_phase);
	}

	return 0;
}

int Watchman::_handle_children()
{
	int i;

	for (i = 0; i < WATCHMAN_MAX_CHILDREN; ++i)
		if (_children[i].child)
			_handle_child(i);

	return 0;
}

int Watchman::_handle_child(int i)
{
	int err;

	if (_pfds[1 + 4*i].revents & POLLIN) {
		err = _children[i].buffer->read_from_stdout(_children[i].child->file_o());
		if (unlikely(err)) {
			/* TODO Handle error.
			 */
		}
	}
	if (_pfds[2 + 4*i].revents & POLLIN) {
		err = _children[i].buffer->read_from_stderr(_children[i].child->file_e());
		if (unlikely(err)) {
			/* TODO Handle error.
			 */
		}
	}
	if (_pfds[3 + 4*i].revents & POLLOUT) {
		err = _children[i].buffer->write_to_stdout(_children[i].fo);
		if (unlikely(err)) {
			/* TODO Handle error.
			 */
		}
	}
	if (_pfds[4 + 4*i].revents & POLLOUT) {
		err = _children[i].buffer->write_to_stderr(_children[i].fe);
		if (unlikely(err)) {
			/* TODO Handle error.
			 */
		}
	}

	if ((WATCHMAN_CHILD_FINISHED == _children[i].flags) &&
	    (_pfds[1 + 4*i].revents & POLLHUP) &&
	    (_pfds[2 + 4*i].revents & POLLHUP)) {

		err = _children[i].child->wait();
		if (unlikely(err)) {
			/* TODO Handle error.
			 */
		}

		_children[i].child  = NULL;
		_children[i].flags  = 0;
		_children[i].buffer = NULL;
		_children[i].fo     = NULL;
		_children[i].fe     = NULL;
	}

	return 0;
}

int Watchman::add_child(Child *child, Buffer *buffer, File *fo, File *fe)
{
	int i;

	for (i = 0; i < WATCHMAN_MAX_CHILDREN; ++i)
		if (NULL == _children[i].child) {
			_children[i].child  = child;
			_children[i].buffer = buffer;
			_children[i].fo     = fo;
			_children[i].fe     = fe;
			return 0;
		}

	return -ENOMEM;
}

