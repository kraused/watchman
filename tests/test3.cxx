
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include "plugin.hxx"
#include "program.hxx"
#include "buffer.hxx"
#include "file.hxx"
#include "watchman.hxx"
#include "compiler.hxx"
#include "error.hxx"

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

Test3_Program::Test3_Program()
: Program(_fill_argv())
{
}

static Watchman_Plugin	plu;
static Test3_Program	proc;
static Buffer		buf;
static File		fo(STDOUT_FILENO);
static File		fe(STDERR_FILENO);

int _init(Watchman *w, int argc, char **argv)
{
	int err;

	if (2 == argc) {
		err = open(argv[0], O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);
		if (unlikely(err < 0)) {
			WATCHMAN_ERROR("open() failed with errno %d: %s", errno, strerror(errno));
			return -errno;
		}
		fo.replace_fd(err);

		err = open(argv[1], O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);
		if (unlikely(err < 0)) {
			WATCHMAN_ERROR("open() failed with errno %d: %s", errno, strerror(errno));
			return -errno;
		}
		fe.replace_fd(err);
	}

	err = w->add_child(&proc, &buf, &fo, &fe);
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

