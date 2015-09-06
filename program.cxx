
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "program.hxx"
#include "compiler.hxx"
#include "error.hxx"

Program::Program(char **argv)
: _argc(_count_argc(argv))
{
	int i, n;

	for (i = 0; i < (WATCHMAN_PROGRAM_MAX_ARGC + 1); ++i)
		_argv[i] = NULL;

	for (i = 0; i < _argc; ++i) {
		_argv[i] = &_mem[i*(WATCHMAN_PROGRAM_MAX_ARGV_STRLEN + 1)];

		n = strlen(argv[i]);
		if (unlikely(n > WATCHMAN_PROGRAM_MAX_ARGV_STRLEN))
			WATCHMAN_WARN("argv[%d] is too long", i);

		*(char *)mempcpy(_argv[i], argv[i], n) = 0;
	}
}

int Program::execute()
{
	int err;
	int po[2], pe[2];

	err = pipe(po);
	if (unlikely(err)) {
		WATCHMAN_ERROR("pipe() failed with errno %d: %s", errno, strerror(errno));
		return -errno;
	}

	err = pipe(pe);
	if (unlikely(err)) {
		WATCHMAN_ERROR("pipe() failed with errno %d: %s", errno, strerror(errno));
		return -errno;
	}

	_pid = fork();
	if (0 == _pid) {
		close(po[0]);
		close(pe[0]);

		/* FIXME Handle dup2 error. At least report close() errors.
		 */

		if (po[1] != STDOUT_FILENO) {
			dup2 (po[1], STDOUT_FILENO);
			close(po[1]);
		}
		if (pe[1] != STDERR_FILENO) {
			dup2 (pe[1], STDERR_FILENO);
			close(pe[1]);
		}

		_child_exec();
	}

	close(po[1]);
	close(pe[1]);

	_fd_o = po[0];
	_fd_e = pe[0];

	return 0;
}

int Program::_count_argc(char **argv)
{
	int i, n;

	n = 0;
	for (i = 0; argv[i]; ++i)
		++n;

	if (unlikely(n > WATCHMAN_PROGRAM_MAX_ARGC)) {
		WATCHMAN_WARN("argc == %d is too large", n);
		n = WATCHMAN_PROGRAM_MAX_ARGC;
	}

	return n;
}

void Program::_child_exec()
{
	int err;

	err = execv(_argv[0], _argv);
	if (unlikely(err)) {
		WATCHMAN_ERROR("execv() failed with errno %d: %s", errno, strerror(errno));

		exit(1);
	}
}

