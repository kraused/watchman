
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/signalfd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>

#include "compiler.h"
#include "config.h"
#include "failfs.hxx"
#include "error.hxx"

static int _write_string(int fd, const char *str)
{
	int len;
	int err;
	int tmp;

	err = 0;

	len = str ? (strlen(str) + 1) : 0;

	tmp = write(fd, &len, sizeof(int));
	if (unlikely(tmp != (int )sizeof(int))) {
		FAILFS_ERROR("write() failed with errno %d: %s", errno, strerror(errno));
		err = -errno;
	}

	if (len > FAILFS_PATH_MAXLEN) {
		FAILFS_ERROR("Truncating string from %d to %d", len, strlen);
		len = FAILFS_PATH_MAXLEN;
	}

	if (len > 0) {
		tmp = write(fd, str, len);
		if (unlikely(tmp != len)) {
			FAILFS_ERROR("write() failed with errno %d: %s", errno, strerror(errno));
			err = -errno;
		}
	}

	return err;
}

static int _read_string(int fd, char *str, int strlen)
{
	int len;
	int err;
	int tmp;
	char dummy;

	err = 0;

	if (str)
		memset(str, 0, strlen);

	tmp = read(fd, &len, sizeof(int));
	if (unlikely(tmp != (int )sizeof(int))) {
		FAILFS_ERROR("read() failed with errno %d: %s", errno, strerror(errno));
		err = -errno;
	}

	if (len > strlen) {
		FAILFS_ERROR("Truncating string from %d to %d", len, strlen);
		len = strlen;
	}

	if (len > 0) {
		/* Handle the case NULL == str so that we do not run into problems because only a partial
		 * message is read.
		 */
		if (unlikely(!str)) {
			while (len > 0) {
				tmp = read(fd, &dummy, 1);
				if (unlikely(tmp != 1)) {
					FAILFS_ERROR("read() failed with errno %d: %s", errno, strerror(errno));
					err = -errno;
				}
				--len;
			}
		} else {
			tmp = read(fd, str, len);
			if (unlikely(tmp != len)) {
				FAILFS_ERROR("read() failed with errno %d: %s", errno, strerror(errno));
				err = -errno;
			}
		}
	}

	return err;
}

Failfs::Failfs()
: _state(FAILFS_STATE_NORMAL), _sfd(-1), _exit_loop(0), _listenfd(-1), _cmdfd(-1)
{
}

Failfs::~Failfs()
{
}

int Failfs::init_signal_handling()
{
	sigset_t all;
	int err;

	sigfillset(&all);
	err = sigprocmask(SIG_BLOCK, &all, &_default_signal_set);
	if (unlikely(err)) {
		FAILFS_ERROR("Failed to block signals: %s", errno);
		return -errno;
	}

	_sfd = signalfd(-1, &all, 0);
	if (unlikely(-1 == _sfd)) {
		FAILFS_ERROR("Failed to open signalfd: %s", errno);
		return -errno;
	}

	return 0;
}

int Failfs::fini_signal_handling()
{
	int err;

	err = close(_sfd);
	if (unlikely(err)) {
		FAILFS_ERROR("close() failed with errno %d: %s", errno, strerror(errno));
	}

	err = sigprocmask(SIG_BLOCK, &_default_signal_set, NULL);
	if (unlikely(err)) {
		FAILFS_ERROR("Failed to reset default signal set");
		return -errno;
	}

	return 0;
}

int Failfs::loop()
{
	int err;

	while (!_exit_loop) {
		err = _poll();
		if (unlikely(err)) {
			continue;
		}

		if (_pfds[0].revents & POLLIN)
			_recv_and_handle_signal();

		if (_pfds[1].revents & POLLIN)
			_recv_and_handle_message();

		if (_pfds[2].revents & POLLIN)
			_handle_commands();
	}

	return -1;
}

void Failfs::_fill_poll_fds()
{
	memset(_pfds, 0, sizeof(_pfds));

	_pfds[0].fd     = _sfd;
	_pfds[0].events = POLLIN;

	_pfds[1].fd     = _sockp[0];
	_pfds[1].events = POLLIN;

	/* Accept new connections only when
	 */
	if (-1 == _cmdfd) {
		_pfds[2].fd     = _listenfd;
		_pfds[2].events = POLLIN;
	} else {
		_pfds[2].fd     = _cmdfd;
		_pfds[2].events = POLLIN;
	}

	_num_pfds = 3;
}

int Failfs::_poll()
{
	int err;

	_fill_poll_fds();

	err = poll(_pfds, _num_pfds, -1);
	if (unlikely(err < 0)) {
		FAILFS_ERROR("poll failed: %s", errno);
		return -errno;
	}

	return 0;
}

int Failfs::_recv_and_handle_signal()
{
	struct signalfd_siginfo info;
	int err;

	err = read(_sfd, &info, sizeof(info));
	if (unlikely(err < 0)) {
		FAILFS_ERROR("read() failed with errno %d: %s", errno, strerror(errno));
		return -errno;
	}
	if (err != sizeof(info)) {
		FAILFS_ERROR("read() returned %d (sizeof signalfd_siginfo is %d)\n", err, sizeof(info));
		return -1;
	}

	FAILFS_DEBUG("Received signal %d", info.ssi_signo);

	if ((SIGINT  == info.ssi_signo) ||
	    (SIGQUIT == info.ssi_signo) ||
	    (SIGTERM == info.ssi_signo))
	        return _handle_sigquit(info.ssi_signo);
	if ((SIGCHLD == info.ssi_signo))
		return 0;	/* FUSE is forking to exec() fusermount */

	FAILFS_WARN("Ignoring signal %d from pid %d (uid %d)", info.ssi_signo, info.ssi_pid, info.ssi_uid);

	return 0;
}

int Failfs::_handle_sigquit(int signo)
{
	_exit_loop = 1;

	return 0;
}

int Failfs::_recv_and_handle_message()
{
	int msg;
	int err;
	int tmp;

	err = read(_sockp[0], &msg, sizeof(int));
	if (unlikely(err != (int )sizeof(int))) {
		FAILFS_ERROR("read() failed with errno %d: %s", errno, strerror(errno));
		err = -errno;
	}

	FAILFS_DEBUG("Received message %d", msg);

	switch (msg) {
	case FAILFS_NOTIFY_EXIT:
		_exit_loop = 1;
		break;
	case FAILFS_FUSE_CMD_GETATTR:
	case FAILFS_FUSE_CMD_READDIR:
	case FAILFS_FUSE_CMD_ACCESS:
	case FAILFS_FUSE_CMD_CREATE:
	case FAILFS_FUSE_CMD_OPEN:
	case FAILFS_FUSE_CMD_RELEASE:
	case FAILFS_FUSE_CMD_READ:
	case FAILFS_FUSE_CMD_WRITE:
	case FAILFS_FUSE_CMD_UNLINK:
	case FAILFS_FUSE_CMD_TRUNCATE:
	case FAILFS_FUSE_CMD_FTRUNCATE:
		tmp = _recv_and_handle_fuse_cmd(msg);
		if (unlikely(tmp))
			err = tmp;
		break;
	default:
		FAILFS_WARN("Ignoring unknown message %d", msg);
	}

	return err;
}

const char *_translate_fuse_cmd_to_string(int cmd)
{
	switch (cmd) {
	case FAILFS_FUSE_CMD_GETATTR:
		return "getattr";
		break;
	case FAILFS_FUSE_CMD_READDIR:
		return "readdir";
		break;
	case FAILFS_FUSE_CMD_ACCESS:
		return "access";
		break;
	case FAILFS_FUSE_CMD_CREATE:
		return "create";
		break;
	case FAILFS_FUSE_CMD_OPEN:
		return "open";
		break;
	case FAILFS_FUSE_CMD_RELEASE:
		return "release";
		break;
	case FAILFS_FUSE_CMD_READ:
		return "read";
		break;
	case FAILFS_FUSE_CMD_WRITE:
		return "write";
		break;
	case FAILFS_FUSE_CMD_UNLINK:
		return "unlink";
		break;
	case FAILFS_FUSE_CMD_TRUNCATE:
		return "truncate";
		break;
	case FAILFS_FUSE_CMD_FTRUNCATE:
		return "ftruncate";
		break;
	}

	return "(null)";
}

int Failfs::_recv_and_handle_fuse_cmd(int cmd)
{
	int tmp;
	int err;
	int cmd_err_val;

	err = 0;

	tmp = _read_string(_sockp[0], _tmp_buf0, FAILFS_PATH_MAXLEN);
	if (unlikely(tmp)) {
		err = tmp;
	}

	cmd_err_val = -1;
	switch (_state) {
	case FAILFS_STATE_NORMAL:
		cmd_err_val = 0;
		break;
	case FAILFS_STATE_STALE:
		cmd_err_val = -ESTALE;
		break;
	default:
		FAILFS_ERROR("Unknown state %d", _state);
	}

	if (strlen(_tmp_buf0) > 0) {
		tmp = _translate_path(_tmp_buf0, _tmp_buf1);
		if (unlikely(tmp)) {
			cmd_err_val = tmp;
		}
	} else {
		memset(_tmp_buf1, 0, FAILFS_PATH_MAXLEN);
	}

	tmp = write(_sockp[0], &cmd_err_val, sizeof(int));
	if (unlikely(tmp != (int )sizeof(int))) {
		FAILFS_ERROR("write() failed with errno %d: %s", errno, strerror(errno));
		err = -errno;
	}

	if (likely(0 == cmd_err_val)) {
		tmp = _write_string(_sockp[0], _tmp_buf1);
		if (unlikely(tmp)) {
			err = tmp;
		}
	}

	FAILFS_DEBUG("%s: input-path = \"%s\", translated-path = \"%s\", cmd_err_val = %d",
	             _translate_fuse_cmd_to_string(cmd),
	             _tmp_buf0, _tmp_buf1,
	             cmd_err_val);

	return err;
}

int Failfs::create_mirrordir()
{
	char pid[32];
	int err;

	err = snprintf(pid, sizeof(pid), "%d", (int )getpid());
	if (unlikely((err < 0) || (err >= (int )sizeof(pid)))) {
		FAILFS_ERROR("snprintf() failed: pid truncated");
	}
	snprintf(_mirrordir, FAILFS_PATH_MAXLEN, FAILFS_MIRRORDIR, pid);
	if (unlikely((err < 0) || (err >= FAILFS_PATH_MAXLEN))) {
		FAILFS_ERROR("snprintf() failed: mirrordir truncated");
	}

	err = 0;

	err = mkdir(_mirrordir, S_IRWXU);
	if (unlikely(err)) {
		FAILFS_ERROR("mkdir() failed with errno %d: %s", errno, strerror(errno));
		err = -errno;
	}

	return err;
}

int Failfs::remove_mirrordir()
{
	/* FIXME Remove the mirrordir */

	return -1;
}

int Failfs::create_socketpair()
{
	int err;

	err = socketpair(AF_UNIX, SOCK_STREAM, 0, _sockp);
	if (unlikely(err < 0)) {
		FAILFS_ERROR("socketpair() failed with errno %d: %s", errno, strerror(errno));
		return -errno;
	}

	return 0;
}

int Failfs::close_socketpair()
{
	int err;

	err = close(_sockp[0]);
	if (unlikely(err)) {
		FAILFS_ERROR("close() failed with errno %d: %s", errno, strerror(errno));
	}

	err = close(_sockp[1]);
	if (unlikely(err)) {
		FAILFS_ERROR("close() failed with errno %d: %s", errno, strerror(errno));
	}

	return 0;
}

int Failfs::send_fuse_cmd_and_recv(int cmd,
                                   const char *path,
                                   char *mirror_path,
                                   int *cmd_err)
{
	int msg = cmd;
	int err;
	int tmp;
	int cmd_err_val;

	err = 0;

	tmp = write(_sockp[1], &msg, sizeof(int));
	if (unlikely(tmp != (int )sizeof(int))) {
		FAILFS_ERROR("write() failed with errno %d: %s", errno, strerror(errno));
		err = -errno;
	}

	tmp = _write_string(_sockp[1], path);
	if (unlikely(err)) {
		err = tmp;
	}

	tmp = read(_sockp[1], &cmd_err_val, sizeof(int));
	if (unlikely(tmp != (int )sizeof(int))) {
		FAILFS_ERROR("read() failed with errno %d: %s", errno, strerror(errno));
		err = -errno;
	}

	if (likely(cmd_err)) {
		*cmd_err = cmd_err_val;
	}

	if (unlikely(!mirror_path)) {
		FAILFS_ERROR("Invalid 3rd argument.");
		err = -EINVAL;
	}

	if (likely(0 == cmd_err_val)) {
		tmp = _read_string(_sockp[1], mirror_path, FAILFS_PATH_MAXLEN);
		if (unlikely(err)) {
			err = tmp;
		}
	}

	return err;
}

int Failfs::send_exit_notification()
{
	int msg = FAILFS_NOTIFY_EXIT;
	int err;

	err = write(_sockp[1], &msg, sizeof(int));
	if (unlikely(err != (int )sizeof(int))) {
		FAILFS_ERROR("write() failed with errno %d: %s", errno, strerror(errno));
		return -errno;
	}

	return 0;
}

int Failfs::_translate_path(const char *i_path, char *o_path)
{
	int err;

	err = snprintf(o_path, FAILFS_PATH_MAXLEN, "%s%s", _mirrordir, i_path);
	if (unlikely((err < 0) || (err >= FAILFS_PATH_MAXLEN))) {
		return -ENAMETOOLONG;
	}

	return 0;
}

int Failfs::bind_listenfd(const char *path)
{
	int err;
	struct sockaddr_un addr;

	_listenfd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (unlikely(_listenfd < 0)) {
		FAILFS_ERROR("socket() failed with errno %d: %s", errno, strerror(errno));
		return -errno;
	}

	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;

	err = snprintf(_listenpath, FAILFS_PATH_MAXLEN, "%s", path);
	if (unlikely((err < 0) || (err >= FAILFS_PATH_MAXLEN))) {
		FAILFS_ERROR("snprintf() failed: unix domain socket path truncated");
		return -ENAMETOOLONG;
	}

	err = snprintf(addr.sun_path, sizeof(addr.sun_path), "%s", path);
	if (unlikely((err < 0) || (err >= FAILFS_PATH_MAXLEN))) {
		FAILFS_ERROR("snprintf() failed: unix domain socket path truncated");
		return -ENAMETOOLONG;
	}

	err = bind(_listenfd, (const struct sockaddr *)&addr, sizeof(addr));
	if (unlikely(err < 0)) {
		FAILFS_ERROR("bind() failed with errno %d: %s", errno, strerror(errno));
		return -errno;
	}

	err = listen(_listenfd, 4);
	if (unlikely(err < 0)) {
		FAILFS_ERROR("listen() failed with errno %d: %s", errno, strerror(errno));
		return -errno;
	}

	return 0;
}

int Failfs::close_listenfd()
{
	int err;

	err = 0;

	err = close(_listenfd);
	if (unlikely(err)) {
		FAILFS_ERROR("close() failed with errno %d: %s", errno, strerror(errno));
		err = -errno;
	}

	err = unlink(_listenpath);
	if (unlikely(err)) {
		FAILFS_ERROR("unlink() failed with errno %d: %s", errno, strerror(errno));
		err = -errno;
	}

	return err;
}

int Failfs::_handle_commands()
{
	int err;
	int cmd;

	if (-1 == _cmdfd) {
		_cmdfd = accept(_listenfd, NULL, NULL);
		if (unlikely(-1 == _cmdfd)) {
			FAILFS_ERROR("accept() failed with errno %d: %s", errno, strerror(errno));
			return -errno;
		}
	} else {
		err = read(_cmdfd, &cmd, sizeof(int));
		if (unlikely(err != (int )sizeof(int))) {
			FAILFS_ERROR("read() failed with errno %d: %s", errno, strerror(errno));
		}

		FAILFS_DEBUG("Received external command %d", cmd);

		switch (cmd) {
		case FAILFS_CMD_CHANGE_STATE_STALE:
			_state = FAILFS_STATE_STALE;
			break;
		case FAILFS_CMD_CHANGE_STATE_NORMAL:
			_state = FAILFS_STATE_NORMAL;
			break;
		default:
			FAILFS_WARN("Ignoring unknown command %d", cmd);
		}

		err = close(_cmdfd);
		if (unlikely(err)) {
			FAILFS_ERROR("close() failed with errno %d: %s", errno, strerror(errno));
		}
		_cmdfd = -1;
		FAILFS_DEBUG("Closed the command file descriptor.");
	}

	return 0;
}

int Failfs_send_fuse_cmd_and_recv(void *failfs,
                                  int cmd,
                                  const char *path,
                                  char *mirror_path,
                                  int *cmd_err)
{
	return ((Failfs *)failfs)->send_fuse_cmd_and_recv(cmd, path, mirror_path, cmd_err);
}

