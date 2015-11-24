
#ifndef WATCHMAN_WATCHMAN_HXX_INCLUDED
#define WATCHMAN_WATCHMAN_HXX_INCLUDED 1

#include <signal.h>
#include <poll.h>

#include "config.hxx"

class Child;
class Buffer;
class File_Pair;

/* Watchman: Main application class.
 */
class Watchman
{

public:
			explicit Watchman();
			~Watchman();

public:
	int		loop();

private:
	int		_sfd;
	sigset_t	_default_signal_set;

public:
			/* Initialize the signal handling module. Block
			 * signals and open the signalfd.
			 */
	int		init_signal_handling();
	int		fini_signal_handling();

			/* Call execute() for all children.
			 */
	int		execute_children();

private:
	int		_recv_and_handle_signal();

	int		_handle_sigchld(long long pid);
	int		_handle_sigquit(int signo);
	int		_handle_sigalrm();

private:
	Child		*_children[WATCHMAN_MAX_CHILDREN];
	Buffer		*_buffers [WATCHMAN_MAX_CHILDREN];
	File_Pair	*_fps     [WATCHMAN_MAX_CHILDREN];

	int		_num_children_left() const;
	int		_find_child_by_pid(long long pid) const;

	int		_handle_children();
	int		_handle_child(int i);

public:
			/* Add a new children to the list. To each children an associated
			 * buffer instance and output file pair is associated. The buffer
			 * and file pair maybe the same for different children. Mixing the
			 * same buffer structure with different file pairs however is not
			 * advised as it will result in an unpredicable distribution of
			 * output lines over the different files.
			 */
	int		add_child(Child *child, Buffer *buffer, File_Pair *fp);

private:
#undef	WATCHMAN_MAX_POLLFDS
#define	WATCHMAN_MAX_POLLFDS	(1 + 4*WATCHMAN_MAX_CHILDREN)

	int		_num_pfds;
	struct pollfd	_pfds[WATCHMAN_MAX_POLLFDS];

	void		_fill_poll_fds();
	int		_poll();

private:
	int		_exit_phase;

};

enum
{
	WATCHMAN_EXIT_PHASE_CONT = 0,
	WATCHMAN_EXIT_PHASE_BEGIN,
	WATCHMAN_EXIT_PHASE_AGAIN,
	WATCHMAN_EXIT_PHASE_QUIT
};

#endif

