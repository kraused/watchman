
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "named_file.hxx"
#include "compiler.hxx"
#include "error.hxx"

Named_File::Named_File()
: File(-1), _oflags(0), _perms(0)
{
	memset(_path, 0, WATCHMAN_PATH_MAX_LEN);

	_can_reopen = true;
}

int Named_File::open(const char *path, int oflags, int perms)
{
	int err;

	_oflags = oflags;
	_perms  = perms;

	err = snprintf(_path, WATCHMAN_PATH_MAX_LEN, "%s", path);
	if (unlikely((err < 0) || (err >= WATCHMAN_PATH_MAX_LEN))) {
		WATCHMAN_ERROR("snprintf() failed: path truncated");
		return -1;
	}

	_fd = ::open(_path, _oflags, _perms);
	if (unlikely(_fd < 0)) {
		WATCHMAN_ERROR("open() failed with errno %d: %s", errno, strerror(errno));
		return -errno;
	}

	_state = WATCHMAN_FILE_STATE_HEALTHY;

	return 0;
}

int Named_File::reopen()
{
	int err;

	if (_fd > 0) {
		err = ::close(_fd);
		if (unlikely(err < 0)) {
			WATCHMAN_ERROR("close() failed with errno %d: %s", errno, strerror(errno));
		}
		_fd = -1;
	}

	_fd = ::open(_path, _oflags & (~O_TRUNC), _perms);
	if (unlikely(_fd < 0)) {
		WATCHMAN_ERROR("open() failed with errno %d: %s", errno, strerror(errno));
		return -errno;
	}

	err = ::lseek(_fd, 0, SEEK_END);
	if (unlikely(err < 0)) {
		WATCHMAN_ERROR("lseek() failed with errno %d: %s", errno, strerror(errno));
		return -errno;
	}

	_state = WATCHMAN_FILE_STATE_HEALTHY;

	return 0;
}

