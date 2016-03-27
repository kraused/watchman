
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include "watchman/plugin.hxx"
#include "watchman/alloc.hxx"
#include "watchman/program.hxx"
#include "watchman/buffer.hxx"
#include "watchman/file.hxx"
#include "watchman/named_file.hxx"
#include "watchman/watchman.hxx"
#include "watchman/compiler.hxx"
#include "watchman/error.hxx"

static char _producer[6][WATCHMAN_PROGRAM_MAX_ARGV_STRLEN + 1];
static char *_argv[7];

static char **_fill_argv()
{
	strcpy(_producer[0], "tests/producer");
	strcpy(_producer[1], "100");	/* number of rounds */
	strcpy(_producer[2], "1-200");	/* line length variation */
	strcpy(_producer[3], "1-10");	/* number of lines written at once */
	strcpy(_producer[4], "10");	/* output frequency [Hz] */
	strcpy(_producer[5], "tests/test3.copy");

	_argv[0] = _producer[0];
	_argv[1] = _producer[1];
	_argv[2] = _producer[2];
	_argv[3] = _producer[3];
	_argv[4] = _producer[4];
	_argv[5] = _producer[5];
	_argv[6] = NULL;

	return _argv;
}

class Test3_Program : public Program
{

public:
			Test3_Program();

};

class Test3_Plugin : public Watchman_Plugin
{

public:
			explicit Test3_Plugin(void *handle, int version);

public:
	int		init(Watchman *w, int argc, char **argv);
	int		fini();

private:
	Allocator	*_alloc;

private:
	Test3_Program	_proc;

private:
	Buffer		_buf;

private:
	File		*_fo;
	File		*_fe;
};

Test3_Program::Test3_Program()
: Program(_fill_argv())
{
}

Test3_Plugin::Test3_Plugin(void *handle, int version)
: Watchman_Plugin(handle, version), _fo(NULL), _fe(NULL)
{
}

int Test3_Plugin::init(Watchman *w, int argc, char **argv)
{
	Named_File *fd;
	int err;

	_alloc = w->alloc();

	if (2 == argc) {
		fd  = _alloc->create<Named_File>();
		err = fd->open(argv[0], O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);
		if (unlikely(err)) {
			return err;
		}
		_fo = fd;

		fd  = _alloc->create<Named_File>();
		err = fd->open(argv[1], O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);
		if (unlikely(err)) {
			return err;
		}
		_fe = fd;
	} else {
		_fo = _alloc->create<File>(STDOUT_FILENO);
		_fe = _alloc->create<File>(STDERR_FILENO);
	}

	err = w->add_child(&_proc, &_buf, _fo, _fe);
	if (unlikely(err)) {
		WATCHMAN_ERROR("Failed to add children to list: %d", err);
		return err;
	}

	return 0;
}

int Test3_Plugin::fini()
{
	_fo = _alloc->destroy<File>(_fo);
	_fe = _alloc->destroy<File>(_fe);

	return 0;
}

extern "C" Watchman_Plugin *entry(void *handle, Watchman *w)
{
	Allocator *alloc;

	alloc = w->alloc();

	return alloc->create<Test3_Plugin>(handle, 1);
}

