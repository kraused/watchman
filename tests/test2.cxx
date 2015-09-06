
#include <string.h>

#include "plugin.hxx"
#include "program.hxx"
#include "watchman.hxx"
#include "compiler.hxx"
#include "error.hxx"

static char _uname[2][WATCHMAN_PROGRAM_MAX_ARGV_STRLEN + 1];
static char *_argv[3];

static char **_fill_argv()
{
	strcpy(_uname[0], "/usr/bin/uname");
	strcpy(_uname[1], "-a");

	_argv[0] = _uname[0];
	_argv[1] = _uname[1];
	_argv[2] = NULL;

	return _argv;
}

class Test2_Program : public Program
{

public:
			Test2_Program();

};

Test2_Program::Test2_Program()
: Program(_fill_argv())
{
}

static Watchman_Plugin plu;
static Test2_Program     proc;

int _init(Watchman *w)
{
	int err;

	err = w->add_child(&proc);
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

