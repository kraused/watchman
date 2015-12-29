
#ifndef FAILFS_FAILFS_HXX_INCLUDED
#define FAILFS_FAILFS_HXX_INCLUDED 1

#include "config.h"

enum
{
	FAILFS_STATE_NORMAL,
	FAILFS_STATE_STALE
};

/* Message types for external communication.
 */
enum
{
	FAILFS_CMD_NOOP,

	FAILFS_CMD_CHANGE_STATE_STALE,
	FAILFS_CMD_CHANGE_STATE_NORMAL
};

/* Message types for internal communication with the FUSE
 * thread.
 */
enum
{
	FAILFS_NOTIFY_EXIT,

	FAILFS_FUSE_CMD_GETATTR,
	FAILFS_FUSE_CMD_READDIR,
	FAILFS_FUSE_CMD_ACCESS,
	FAILFS_FUSE_CMD_CREATE,
	FAILFS_FUSE_CMD_OPEN,
	FAILFS_FUSE_CMD_RELEASE,
	FAILFS_FUSE_CMD_READ,
	FAILFS_FUSE_CMD_WRITE,
	FAILFS_FUSE_CMD_UNLINK,
	FAILFS_FUSE_CMD_TRUNCATE,
	FAILFS_FUSE_CMD_FTRUNCATE
};


#ifdef __cplusplus

#include <signal.h>
#include <poll.h>

/* Failfs: Main application class.
 */
class Failfs
{

public:
			explicit Failfs();
			~Failfs();

private:
	int		_state;

private:
	int		_sfd;
	sigset_t	_default_signal_set;

public:
	                /* Initialize the signal handling module. Block
	                 * signals and open the signalfd.
	                 */
	int             init_signal_handling();
	int             fini_signal_handling();

private:
	int		_recv_and_handle_signal();
	int		_handle_sigquit(int signo);

private:
	int		_recv_and_handle_message();
	int		_recv_and_handle_fuse_cmd(int cmd);

public:
	int		loop();

private:
	int		_num_pfds;
	struct pollfd	_pfds[4];

	void		_fill_poll_fds();
	int		_poll();

private:
	int		_exit_loop;

private:
			/* Directory mirrored by failfs when operating
			 * normally.
			 */
	char		_mirrordir[FAILFS_PATH_MAXLEN];

public:
	int		create_mirrordir();
	int		remove_mirrordir();

private:
			/* Socket pair for the communication with the FUSE
			 * thread.
			 */
	int		_sockp[2];

public:
	int		create_socketpair();
	int		close_socketpair();

			/* Send a FUSE command message from the FUSE thread to
			 * the main thread and receive the mirrored path as well
			 * an error message if the request is to fail.
			 *
			 * mirror_path must have length > FAILFS_PATH_MAXLEN.
			 */
	int		send_fuse_cmd_and_recv(int cmd,
			                       const char *path,
			                       char *mirror_path,
			                       int *cmd_err);

	int		send_exit_notification();

private:
	char		_tmp_buf0[FAILFS_PATH_MAXLEN];
	char		_tmp_buf1[FAILFS_PATH_MAXLEN];

private:
	int		_translate_path(const char *i_path, char *o_path);

public:
	int		bind_listenfd(const char *path);
	int		close_listenfd();

private:
	int		_handle_commands();

private:
	char		_listenpath[FAILFS_PATH_MAXLEN];
	int		_listenfd;

	int		_cmdfd;

};

#endif

#ifdef __cplusplus
extern "C"
{
#endif

int Failfs_send_fuse_cmd_and_recv(void *failfs,
                                  int cmd,
                                  const char *path,
                                  char *mirror_path,
                                  int *cmd_err);

#ifdef __cplusplus
}
#endif

#endif

