
#include <unistd.h>
#include <string.h>

#include "plugin.hxx"
#include "program.hxx"
#include "buffer.hxx"
#include "file_pair.hxx"
#include "watchman.hxx"
#include "compiler.hxx"
#include "error.hxx"

static char _producer[6][WATCHMAN_PROGRAM_MAX_ARGV_STRLEN + 1];
static char *_argv[7];

static char **_fill_argv()
{
	/* Write a large number of lines at once to make sure that
	 * output that is still pending when the child terminates 
	 * is correctly read.
	 */
	strcpy(_producer[0], "tests/producer");
	strcpy(_producer[1], "1");	/* number of rounds */
	strcpy(_producer[2], "1-200");	/* line length variation */
	strcpy(_producer[3], "1000");	/* number of lines written at once */
	strcpy(_producer[4], "10");	/* output frequency [Hz] */
	strcpy(_producer[5], "tests/test4.copy");

	_argv[0] = _producer[0];
	_argv[1] = _producer[1];
	_argv[2] = _producer[2];
	_argv[3] = _producer[3];
	_argv[4] = _producer[4];
	_argv[5] = _producer[5];
	_argv[6] = NULL;

	return _argv;
}

class Test4_Program : public Program
{

public:
			Test4_Program();

};

Test4_Program::Test4_Program()
: Program(_fill_argv())
{
}

static Watchman_Plugin	plu;
static Test4_Program	proc;
static Buffer		buf;
static File_Pair	fp(STDOUT_FILENO, STDERR_FILENO);

int _init(Watchman *w)
{
	int err;

	err = w->add_child(&proc, &buf, &fp);
	if (unlikely(err)) {
		WATCHMAN_ERROR("Failed to add children to list: %d", err);
		return err;
	}

	return 0;
};

extern "C" Watchman_Plugin *entry()
{
	plu.version = 1;
	plu.init    = _init;

	return &plu;
};

