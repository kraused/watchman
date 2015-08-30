
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/signalfd.h>

#include "watchman.hxx"
#include "compiler.hxx"
#include "error.hxx"

Watchman::Watchman()
: _sfd(-1), _exit_phase(0)
{
	int i;

	for (i = 0; i < WATCHMAN_MAX_CHILDREN; ++i)
		_children[i] = NULL;
}

Watchman::~Watchman()
{
	/* Check that everything is nicely cleaned-up
	 */
}

int Watchman::init_signal_handling()
{
	sigset_t all;
	int err;

	sigfillset(&all);
	err = sigprocmask(SIG_BLOCK, &all, &_default_signal_set);
	if (unlikely(err)) {
		WATCHMAN_ERROR("Failed to block signals: %s", errno);
		return err;
	}

	_sfd = signalfd(-1, &all, 0);
	if (unlikely(-1 == _sfd)) {
		WATCHMAN_ERROR("Failed to open signalfd: %s", errno);
		return -1;
	}

	return 0;
}

int Watchman::fini_signal_handling()
{
	close(_sfd);

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

void Watchman::_fill_poll_fds()
{
	memset(_pfds, 0, 1*sizeof(struct pollfd));

	_pfds[0].fd     = _sfd;
	_pfds[0].events = POLLIN;
}

int Watchman::_poll()
{
	int err;

	_fill_poll_fds();

	err = poll(_pfds, 1, WATCHMAN_POLL_TIMEOUT);
	if (unlikely(err < 0)) {
		WATCHMAN_ERROR("poll failed: %s", errno);
		return -errno;
	}

	return 0;
}

int Watchman::_recv_and_handle_signal()
{
	struct signalfd_siginfo info;

	read(_sfd, &info, sizeof(info));	/* FIXME Error handling. */

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
		n += (NULL != _children[i]);

	return n;
}

int Watchman::_find_child_by_pid(long long pid) const
{
	int i, j;

	for (i = 0, j = -1; i < WATCHMAN_MAX_CHILDREN; ++i)
		if (_children[i] && (pid == _children[i]->pid())) {
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

	/* FIXME Read remaining bytes from the children stdin/stdout if possible
	 */

	/* FIXME Cleanup
	 */

	_children[i] = NULL;

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
		if (_children[i]) {
			WATCHMAN_LOG("Terminating child process %d", _children[i]->pid());
			_children[i]->terminate();
		}

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
			if (_children[i]) {
				WATCHMAN_LOG("Killing process %d", _children[i]->pid());
				_children[i]->kill();
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
		if (_children[i])
			_handle_child(i);

	return 0;
}

int Watchman::_handle_child(int i)
{
	return 0;
}

int Watchman::add_child(Child *child)
{
	int i;

	for (i = 0; i < WATCHMAN_MAX_CHILDREN; ++i)
		if (NULL == _children[i]) {
			_children[i] = child;
			return 0;
		}

	return -ENOMEM;
}

