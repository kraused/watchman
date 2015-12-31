
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "file.hxx"
#include "compiler.hxx"
#include "error.hxx"

File::File(int fd)
: _fd(fd), _can_reopen(false), _read_err(0), _write_err(0)
{
}

long long File::read(void *buf, long long nbyte)
{
	long long x;

	if (unlikely(_fd < 0))
		return -EBADF;

	x = ::read(_fd, buf, nbyte);
	if (unlikely(x < 0)) {
		if (0 == _read_err) {
			WATCHMAN_ERROR("read() failed with errno %d: %s", errno, strerror(errno));
		}
		++_read_err;

		return -errno;
	}

	return x;
}

long long File::write(const void *buf, long long nbyte)
{
	long long x;

	x = ::write(_fd, buf, nbyte);
	if (unlikely(x < 0)) {
		if (0 == _write_err) {
			WATCHMAN_ERROR("write() failed with errno %d: %s", errno, strerror(errno));
		}
		++_write_err;

		return -errno;
	}

	return x;
}

int File::reopen()
{
	return -1;
}

