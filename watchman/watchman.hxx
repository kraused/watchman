
#ifndef WATCHMAN_WATCHMAN_HXX_INCLUDED
#define WATCHMAN_WATCHMAN_HXX_INCLUDED 1

#include <signal.h>
#include <poll.h>

#include "watchman/config.hxx"

class Allocator;
class Child;
class Buffer;
class File;
class Rotator;

struct _Watchman_Child
{
	Child		*child;
	int		flags;
	Buffer		*buffer;
	File		*fo;
	File		*fe;
	Rotator		*rot;
};

/* Watchman: Main application class.
 */
class Watchman
{

public:
			explicit Watchman(Allocator *alloc);
			~Watchman();

public:
	int		loop();

public:
	Allocator	*alloc();

private:
	Allocator	*_alloc;

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
	_Watchman_Child	_children[WATCHMAN_MAX_CHILDREN];

	int		_num_children_left() const;
	int		_find_child_by_pid(long long pid) const;

	int		_handle_children();
	int		_handle_child(int i);

private:
	int 		_maybe_rotate_files();
	int 		_maybe_rotate_file(int i, File *f, Rotator *rot);

public:
			/* Add a new children to the list. To each children an associated
			 * buffer instance and output file pair is associated. The buffer
			 * and file pair maybe the same for different children. Mixing the
			 * same buffer structure with different file pairs however is not
			 * advised as it will result in an unpredicable distribution of
			 * output lines over the different files.
			 */
	int		add_child(Child *child, Buffer *buffer, File *fo, File *fe, Rotator *rot);

private:
#undef	WATCHMAN_MAX_POLLFDS
#define	WATCHMAN_MAX_POLLFDS	(1 + 4*WATCHMAN_MAX_CHILDREN)

	int		_num_pfds;
	struct pollfd	_pfds[WATCHMAN_MAX_POLLFDS];

	void		_fill_pollfds();
	void		_fill_single_pollfd(struct pollfd *pfd, File *f, int events);
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

enum
{
	WATCHMAN_CHILD_ALIVE	= 1,
	WATCHMAN_CHILD_FINISHED
};

#endif

