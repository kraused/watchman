
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "named_clingy_file.hxx"
#include "compiler.hxx"
#include "error.hxx"

static int _seek_to_string(int fd, const char *str)
{
	long long i;
	long long n = strlen(str);
	char c;
	int err;

	i = 0;
	while (1) {
		err = read(fd, &c, 1);
		if (0 == err)
			break;
		if (unlikely(err < 0)) {
			WATCHMAN_ERROR("read() failed with errno %d: %s", errno, strerror(errno));
			return -1;
		}

		if (c == str[i]) {
			++i;
			if (n == i)
				return 0;
		} else {
			i = 0;
		}
	}

	return -1;
}

static int _copy_until_whitespace(int fd, char *str, long long len)
{
	long long i;
	char c;
	int err;

	for (i = 0; i < len; ++i) {
		err = read(fd, &c, 1);
		if (0 == err)
			break;
		if (unlikely(err < 0)) {
			WATCHMAN_ERROR("read() failed with errno %d: %s", errno, strerror(errno));
			return -1;
		}

		str[i] = c;
		if (' ' == c) {
			str[i] = 0;
			return 0;
		}
	}

	return -1;
}

static int _compare_until_whitespace(int fd, char *str, long long len)
{
	long long i;
	char c;
	int err;

	for (i = 0; i < len; ++i) {
		if (0 == str[i])
			return 0;	/* Complete string matched. */

		err = read(fd, &c, 1);
		if (0 == err)
			break;
		if (unlikely(err < 0)) {
			WATCHMAN_ERROR("read() failed with errno %d: %s", errno, strerror(errno));
			return -1;
		}

		if (c != str[i])
			return 1;
	}

	return -1;
}

Named_Clingy_File::Named_Clingy_File()
{
	memset(_mountpoint, 0, WATCHMAN_PATH_MAX_LEN);
	memset(_fstype, 0, WATCHMAN_FILESYSTEM_MAX_NAME_LEN);
	memset(_source, 0, WATCHMAN_PATH_MAX_LEN);
}

int Named_Clingy_File::attach(const char *mountpoint)
{
	int err;
	int tmp;
	int fd;

	fd = -1;

	err = snprintf(_mountpoint, WATCHMAN_PATH_MAX_LEN, "%s", mountpoint);
	if (unlikely((err < 0) || (err >= WATCHMAN_PATH_MAX_LEN))) {
		WATCHMAN_ERROR("snprintf() failed: mountpoint truncated");
		return -1;
	}

	fd = ::open("/proc/self/mountinfo", O_RDONLY);
	if (unlikely(fd < 0)) {
		WATCHMAN_ERROR("open() failed with errno %d: %s", errno, strerror(errno));
		_mountpoint[0] = 0;
		return -errno;
	}

	err = _copy_from_mountinfo(fd);

	tmp = ::close(fd);
	if (unlikely(tmp < 0)) {
		WATCHMAN_ERROR("close() failed with errno %d: %s", errno, strerror(errno));
	}

	if (unlikely(err)) {
		_mountpoint[0] = 0;
		_fstype[0] = 0;
		_source[0] = 0;

		return err;
	}

	WATCHMAN_DEBUG("mountpoint = \"%s\", fstype = \"%s\", mount source = \"%s\"", \
	               _mountpoint, _fstype, _source);

	return 0;
}

int Named_Clingy_File::_copy_from_mountinfo(int fd)
{
	int err;

	/* fstat() on fd will not return the proper file size (see NOTES section in
	 * man fstat(2)) so we cannot mmap() the file. Reading it char by char is not
	 * very well performing but should suffice for our purposes.
	 */
	err = _seek_to_string(fd, _mountpoint);
	if (unlikely(err < 0)) {
		WATCHMAN_ERROR("Could not find mountpoint in /proc/self/mountinfo");
		return -EINVAL;
	}
	
	err = _seek_to_string(fd, "- ");
	if (unlikely(err < 0)) {
		WATCHMAN_ERROR("Could not find separator in /proc/self/mountinfo");
		return -EINVAL;
	}

	err = _copy_until_whitespace(fd, _fstype, WATCHMAN_FILESYSTEM_MAX_NAME_LEN);
	if (unlikely(err < 0)) {
		WATCHMAN_ERROR("Could not retrieve filesystem type from /proc/self/mountinfo");
		return -EINVAL;
	}
	if (unlikely(err >= WATCHMAN_FILESYSTEM_MAX_NAME_LEN)) {
		WATCHMAN_ERROR("filesystem type truncated");
		/* Try to continue. */
	}

	err = _copy_until_whitespace(fd, _source, WATCHMAN_PATH_MAX_LEN);
	if (unlikely(err < 0)) {
		WATCHMAN_ERROR("Could not retrieve mount source from /proc/self/mountinfo");
		return -EINVAL;
	}
	if (unlikely(err >= WATCHMAN_PATH_MAX_LEN)) {
		WATCHMAN_ERROR("filesystem type truncated");
		/* Try to continue. */
	}

	return 0;
}

bool Named_Clingy_File::can_reopen()
{
	bool mounted;

	if (0 == _mountpoint[0])	/* attach() failed or has not been called. */
		return false;

	mounted = _filesystem_is_mounted();

	return mounted;
}

bool Named_Clingy_File::_filesystem_is_mounted()
{
	int err;
	int tmp;
	int fd;
	bool mounted;

	mounted = true;

	fd = ::open("/proc/self/mountinfo", O_RDONLY);
	if (unlikely(fd < 0)) {
		WATCHMAN_ERROR("open() failed with errno %d: %s", errno, strerror(errno));
		mounted = false;
	}

	err = _seek_to_string(fd, _mountpoint);
	if (unlikely(err < 0)) {
		mounted = false;
		goto exit;
	}
	
	err = _seek_to_string(fd, "- ");
	if (unlikely(err < 0)) {
		mounted = false;
		goto exit;
	}

	err = _compare_until_whitespace(fd, _fstype, WATCHMAN_FILESYSTEM_MAX_NAME_LEN);
	if (unlikely(err))
		mounted = false;
		goto exit;

	err = _compare_until_whitespace(fd, _source, WATCHMAN_PATH_MAX_LEN);
	if (unlikely(err)) {
		mounted = false;
		goto exit;
	}

exit:
	tmp = ::close(fd);
	if (unlikely(tmp < 0)) {
		WATCHMAN_ERROR("close() failed with errno %d: %s", errno, strerror(errno));
	}

	return mounted;
}
