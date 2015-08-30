
#ifndef WATCHMAN_WATCHMAN_HXX_INCLUDED
#define WATCHMAN_WATCHMAN_HXX_INCLUDED 1

#include <signal.h>
#include <poll.h>

#include "config.hxx"
#include "child.hxx"

/* Watchman: Main application class.
 */
class Watchman
{

public:
			explicit Watchman();
			~Watchman();

public:
	int		loop();

public:
	int		_sfd;
	sigset_t	_default_signal_set;

			/* Initialize the signal handling module. Block
			 * signals and open the signalfd.
			 */
	int		init_signal_handling();
	int		fini_signal_handling();

private:
	int		_recv_and_handle_signal();

	int		_handle_sigchld(long long pid);
	int		_handle_sigquit(int signo);
	int		_handle_sigalrm();

private:
	Child		*_children[WATCHMAN_MAX_CHILDREN];

	int		_num_children_left() const;
	int		_find_child_by_pid(long long pid) const;

	int		_handle_children();
	int		_handle_child(int i);

public:
			/* Add a new children to the list.
			 */
	int		add_child(Child *child);

private:
#undef	WATCHMAN_MAX_POLLFDS
#define	WATCHMAN_MAX_POLLFDS	(1 + 2*WATCHMAN_MAX_CHILDREN)

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

