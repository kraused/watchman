
#include <unistd.h>
#include <string.h>

#include "plugin.hxx"
#include "alloc.hxx"
#include "program.hxx"
#include "buffer.hxx"
#include "file.hxx"
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
			explicit Test2_Program();

};

class Test2_Plugin : public Watchman_Plugin
{

public:
			explicit Test2_Plugin(void *handle, int version);

public:
	int		init(Watchman *w, int argc, char **argv);
	int		fini();

private:
	Allocator	*_alloc;

private:
	Test2_Program	_proc;

private:
	Buffer		_buf;

private:
	File		*_fo;
	File		*_fe;

};

Test2_Program::Test2_Program()
: Program(_fill_argv())
{
}

Test2_Plugin::Test2_Plugin(void *handle, int version)
: Watchman_Plugin(handle, version), _fo(NULL), _fe(NULL)
{
}

int Test2_Plugin::init(Watchman *w, int argc, char **argv)
{
	int err;

	_alloc = w->alloc();

	_fo = w->alloc()->create<File>(STDOUT_FILENO);
	_fe = w->alloc()->create<File>(STDERR_FILENO);

	err = w->add_child(&_proc, &_buf, _fo, _fe);
	if (unlikely(err)) {
		WATCHMAN_ERROR("Failed to add children to list: %d", err);
		return err;
	}

	return 0;
}

int Test2_Plugin::fini()
{
	_fo = _alloc->destroy<File>(_fo);
	_fe = _alloc->destroy<File>(_fe);

	return 0;
}

extern "C" Watchman_Plugin *entry(void *handle, Watchman *w)
{
	Allocator *alloc;

	alloc = w->alloc();

	return alloc->create<Test2_Plugin>(handle, 1);
}

