
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "file.hxx"
#include "compiler.hxx"
#include "error.hxx"

/*
static const char *_state_names[] =
{
	[WATCHMAN_FILE_STATE_CLOSED] = "CLOSED",
	[WATCHMAN_FILE_STATE_HEALTHY] = "HEALTHY",
	[WATCHMAN_FILE_STATE_UNHEALTHY] = "UNHEALTHY",
	[WATCHMAN_FILE_STATE_STALE] = "STALE"
};
*/

static const char *_state_to_string(int state)
{
	/* No non-trivial designated initializers supported so we go for this:
	 */
	switch (state) {
	case WATCHMAN_FILE_STATE_CLOSED:
		return "CLOSED";
		break;
	case WATCHMAN_FILE_STATE_HEALTHY:
		return "HEALTHY";
		break;
	case WATCHMAN_FILE_STATE_UNHEALTHY:
		return "UNHEALTHY";
		break;
	case WATCHMAN_FILE_STATE_STALE:
		return "STALE";
		break;
	default:
		WATCHMAN_ERROR("Unknown state %d in _state_to_string().", state);
	}

	return "";
}


File::File(int fd)
: _state(WATCHMAN_FILE_STATE_CLOSED), _fd(fd), _clean(true), _last_write_failed(0)
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
			WATCHMAN_WARN("File %p transitions from %s to %s", this,
				_state_to_string(_state), _state_to_string(WATCHMAN_FILE_STATE_UNHEALTHY));
			_state = WATCHMAN_FILE_STATE_UNHEALTHY;
		}

		if ((ENOTCONN == errno) || (ESTALE == errno)) {
			if (WATCHMAN_FILE_STATE_STALE != _state) {
				WATCHMAN_WARN("File %p transitions from %s to %s", this,
					_state_to_string(_state), _state_to_string(WATCHMAN_FILE_STATE_STALE));
			}
			_state = WATCHMAN_FILE_STATE_STALE;
		}

		return -errno;
	} else {
		if (unlikely(WATCHMAN_FILE_STATE_STALE == _state)) {
			WATCHMAN_WARN("Unexpected state transition %s to %s for file %p",
				_state_to_string(_state), _state_to_string(WATCHMAN_FILE_STATE_HEALTHY), this);
		}

		_state = WATCHMAN_FILE_STATE_HEALTHY;
	}

	increment_file_size(x);

	return x;
}

const char *File::path() const
{
	return nullptr;
}

long long File::size() const
{
	return -1;
}

void File::increment_file_size(long long n)
{
}

bool File::supports_reopen() const
{
	return false;	/* reopen() not implemented */
}

bool File::can_reopen()
{
	return false;
}

int File::reopen(const char *path)
{
	return -1;
}

bool File::supports_rename() const
{
	return false;
}

bool File::can_rename()
{
	return false;
}

int File::rename(const char *newpath)
{
	return -1;
}

void File::flag_as_dirty()
{
	_clean = false;
}

void File::flag_as_clean()
{
	_clean = true;
}

bool File::is_clean() const
{
	return _clean;
}

void File::force_different_state(int state)
{
	if (unlikely((WATCHMAN_FILE_STATE_CLOSED    != state) &&
	             (WATCHMAN_FILE_STATE_HEALTHY   != state) &&
	             (WATCHMAN_FILE_STATE_UNHEALTHY != state) &&
	             (WATCHMAN_FILE_STATE_STALE     != state))) {
		WATCHMAN_ERROR("Rejecting attempt to set file %f state to unknown %d.", this, state);
	}

	WATCHMAN_WARN("Forcing state transition %s to %s for file %p. "
	              "This indicates an application bug.", _state_to_string(_state), _state_to_string(state), this);

	_state = state;
}

