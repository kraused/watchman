
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include "plugin.hxx"
#include "alloc.hxx"
#include "program.hxx"
#include "buffer.hxx"
#include "file.hxx"
#include "named_clingy_file.hxx"
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
	strcpy(_producer[5], "tests/test6.copy");

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

class Test4_Plugin : public Watchman_Plugin
{

public:
				explicit Test4_Plugin(void *handle, int version);

public:
	int			init(Watchman *w, int argc, char **argv);
	int			fini();

private:
	Allocator		*_alloc;

private:
	Test4_Program		_proc;

private:
	Buffer			_buf;

private:
	Named_Clingy_File	*_fo;
	Named_Clingy_File	*_fe;
};

Test4_Program::Test4_Program()
: Program(_fill_argv())
{
}

Test4_Plugin::Test4_Plugin(void *handle, int version)
: Watchman_Plugin(handle, version), _fo(NULL), _fe(NULL)
{
}

int Test4_Plugin::init(Watchman *w, int argc, char **argv)
{
	int err;

	_alloc = w->alloc();

	_fo = _alloc->create<Named_Clingy_File>();
	err = _fo->attach(argv[0]);
	if (unlikely(err)) {
		return err;
	}
	err = _fo->open(argv[1], O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);
	if (unlikely(err)) {
		return err;
	}

	_fe = _alloc->create<Named_Clingy_File>();
	err = _fe->attach(argv[0]);
	if (unlikely(err)) {
		return err;
	}
	err = _fe->open(argv[2], O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);
	if (unlikely(err)) {
		return err;
	}

	err = w->add_child(&_proc, &_buf, _fo, _fe);
	if (unlikely(err)) {
		WATCHMAN_ERROR("Failed to add children to list: %d", err);
		return err;
	}

	return 0;
}

int Test4_Plugin::fini()
{
	_fo = _alloc->destroy<Named_Clingy_File>(_fo);
	_fe = _alloc->destroy<Named_Clingy_File>(_fe);

	return 0;
}

extern "C" Watchman_Plugin *entry(void *handle, Watchman *w)
{
	Allocator *alloc;

	alloc = w->alloc();

	return alloc->create<Test4_Plugin>(handle, 1);
};
