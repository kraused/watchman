
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
: File(-1), _size(0), _oflags(0), _perms(0)
{
	memset(_path, 0, WATCHMAN_PATH_MAX_LEN);
}

static long long _file_size(int fd)
{
	long long err, cur, size;

	err = ::lseek(fd, 0, SEEK_CUR);
	if (unlikely(err < 0)) {
		WATCHMAN_ERROR("lseek() failed with errno %d: %s", errno, strerror(errno));
		return -errno;
	}
	cur = err;

	err = ::lseek(fd, 0, SEEK_END);
	if (unlikely(err < 0)) {
		WATCHMAN_ERROR("lseek() failed with errno %d: %s", errno, strerror(errno));
		return -errno;
	}
	size = err;

	err = ::lseek(fd, cur, SEEK_SET);
	if (unlikely(err < 0)) {
		WATCHMAN_ERROR("lseek() failed with errno %d: %s", errno, strerror(errno));
		return -errno;
	}

	return size;
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

	_size = _file_size(_fd);
	if (unlikely(_size < 0)) {
		WATCHMAN_ERROR("Failed to retrieve file size: %d", _size);
		_state = WATCHMAN_FILE_STATE_UNHEALTHY;
		_size  = 0;
	}

	return 0;
}

const char* Named_File::path() const
{
	return _path;
}

long long Named_File::size() const
{
	return _size;
}

void Named_File::increment_file_size(long long n)
{
	_size += n;
}

bool Named_File::supports_reopen() const
{
	return true;
}

bool Named_File::can_reopen()
{
	return true;
}

int Named_File::reopen(const char *path)
{
	int err;

	if (_fd > 0) {
		err = ::close(_fd);
		if (unlikely(err < 0)) {
			WATCHMAN_ERROR("close() failed with errno %d: %s", errno, strerror(errno));
		}

		_fd    = -1;
		_state = WATCHMAN_FILE_STATE_CLOSED;
	}

	if (path) {
		WATCHMAN_DEBUG("Changing path of file %p from \"%s\" to \"%s\"", this, _path, path);
		err = snprintf(_path, WATCHMAN_PATH_MAX_LEN, "%s", path);
		if (unlikely((err < 0) || (err >= WATCHMAN_PATH_MAX_LEN))) {
			WATCHMAN_ERROR("snprintf() failed: path truncated");
			return -1;
		}
	}

	_fd = ::open(_path, _oflags & (~O_TRUNC), _perms);
	if (unlikely(_fd < 0)) {
		WATCHMAN_ERROR("open() failed with errno %d: %s", errno, strerror(errno));
		return -errno;
	}

	_size = ::lseek(_fd, 0, SEEK_END);
	if (unlikely(_size < 0)) {
		WATCHMAN_ERROR("lseek() failed with errno %d: %s", errno, strerror(errno));
		_state = WATCHMAN_FILE_STATE_UNHEALTHY;

		return -errno;
	}

	if (likely(WATCHMAN_FILE_STATE_HEALTHY != _state)) {
		WATCHMAN_WARN("File %p transitions to HEALTHY", this);
	}

	_state = WATCHMAN_FILE_STATE_HEALTHY;

	return 0;
}

bool Named_File::supports_rename() const
{
	return true;
}

bool Named_File::can_rename()
{
	return true;
}

int Named_File::rename(const char *newpath)
{
	int err;

	if (unlikely(strlen(newpath) >= WATCHMAN_PATH_MAX_LEN))	{
		WATCHMAN_ERROR("new path is too long");
		return -EINVAL;
	}

	err = ::rename(_path, newpath);
	if (unlikely(_fd < 0)) {
		WATCHMAN_ERROR("open() failed with errno %d: %s", errno, strerror(errno));
		return -errno;
	}

	WATCHMAN_DEBUG("Changing path of file %p from \"%s\" to \"%s\"", this, _path, newpath);
	err = snprintf(_path, WATCHMAN_PATH_MAX_LEN, "%s", newpath);
	if (unlikely((err < 0) || (err >= WATCHMAN_PATH_MAX_LEN))) {
		WATCHMAN_ERROR("snprintf() failed: path truncated");
		return -1;
	}

	return 0;
}
