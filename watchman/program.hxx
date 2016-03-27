
#ifndef WATCHMAN_PROGRAM_HXX_INCLUDED
#define WATCHMAN_PROGRAM_HXX_INCLUDED 1

#include "watchman/config.hxx"
#include "watchman/child.hxx"

class Program : public Child
{

public:
				explicit Program(char **argv);

	int			execute();

private:
	static int		_count_argc(char **argv);

private:
	int			_argc;
				/* execv() and friends want pointers to strings so we use a separate
				 * array of pointers.
				 */
	char			*_argv[WATCHMAN_PROGRAM_MAX_ARGC + 1];

	char			_mem[WATCHMAN_PROGRAM_MAX_ARGC*(WATCHMAN_PROGRAM_MAX_ARGV_STRLEN + 1)];

private:
	void			_child_exec();

};

#endif

