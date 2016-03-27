
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "file.hxx"
#include "compiler.hxx"
#include "error.hxx"

File::File(int fd)
: _state(WATCHMAN_FILE_STATE_CLOSED), _fd(fd), _last_write_failed(0)
{
	if (fd >= 0) {
		_state = WATCHMAN_FILE_STATE_HEALTHY;
	}
}

long long File::read(void *buf, long long nbyte)
{
	long long x;

	if (unlikely(_fd < 0))
		return -EBADF;

	x = ::read(_fd, buf, nbyte);
	if (unlikely(x < 0)) {
		WATCHMAN_ERROR("read() failed with errno %d: %s", errno, strerror(errno));

		return -errno;
	}

	return x;
}

long long File::write(const void *buf, long long nbyte)
{
	long long x;

	x = ::write(_fd, buf, nbyte);
	if (unlikely(x < 0)) {
		if (0 == _last_write_failed) {
			WATCHMAN_ERROR("write() failed with errno %d: %s", errno, strerror(errno));
			_last_write_failed = 1;
		}

		if (WATCHMAN_FILE_STATE_HEALTHY == _state) {
			WATCHMAN_WARN("File %p transitions from HEALTHY to UNHEALTY", this);
			_state = WATCHMAN_FILE_STATE_UNHEALTHY;
		}

		if ((ENOTCONN == errno) || (ESTALE == errno)) {
			if (WATCHMAN_FILE_STATE_STALE != _state) {
				WATCHMAN_WARN("File %p transitions to STALE", this);
			}
			_state = WATCHMAN_FILE_STATE_STALE;
		}

		return -errno;
	} else {
		if (unlikely(WATCHMAN_FILE_STATE_STALE == _state)) {
			WATCHMAN_WARN("Unexpected state transition STALE to HEALTHY for file %p", this);
		}

		_state = WATCHMAN_FILE_STATE_HEALTHY;
	}

	return x;
}

bool File::can_reopen()
{
	return false;	/* reopen() not implemented */
}

int File::reopen()
{
	return -1;
}

