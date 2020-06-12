
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "clingy_file.hxx"
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

static int _compare_content(int fd, const char *str, long long len)
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

static int _skip_whitespace(int fd)
{
	char c;
	int err;

	err = read(fd, &c, 1);
	if (0 == err) {
		WATCHMAN_ERROR("read() returned zero.");
		return -1;
	}
	if (unlikely(err < 0)) {
		WATCHMAN_ERROR("read() failed with errno %d: %s", errno, strerror(errno));
		return -1;
	}

	if (' ' == c) {
		return 0;
	}

	return -1;
}

static bool _copy_from_mountinfo(int fd, const char *mountpoint, char *fstype, char *source)
{
	int err;

	/* fstat() on fd will not return the proper file size (see NOTES section in
	 * man fstat(2)) so we cannot mmap() the file. Reading it char by char is not
	 * very well performing but should suffice for our purposes.
	 */
	err = _seek_to_string(fd, mountpoint);
	if (unlikely(err < 0)) {
		WATCHMAN_ERROR("Could not find mountpoint in /proc/self/mountinfo");
		return -EINVAL;
	}
	
	err = _seek_to_string(fd, "- ");
	if (unlikely(err < 0)) {
		WATCHMAN_ERROR("Could not find separator in /proc/self/mountinfo");
		return -EINVAL;
	}

	err = _copy_until_whitespace(fd, fstype, WATCHMAN_FILESYSTEM_MAX_NAME_LEN);
	if (unlikely(err < 0)) {
		WATCHMAN_ERROR("Could not retrieve filesystem type from /proc/self/mountinfo");
		return -EINVAL;
	}
	if (unlikely(err >= WATCHMAN_FILESYSTEM_MAX_NAME_LEN)) {
		WATCHMAN_ERROR("filesystem type truncated");
		/* Try to continue. */
	}

	err = _copy_until_whitespace(fd, source, WATCHMAN_PATH_MAX_LEN);
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

void _clingy_file_clean(char *mountpoint, char *fstype, char *source)
{
	memset(mountpoint, 0, WATCHMAN_PATH_MAX_LEN);
	memset(fstype, 0, WATCHMAN_FILESYSTEM_MAX_NAME_LEN);
	memset(source, 0, WATCHMAN_PATH_MAX_LEN);	
}

bool _clingy_file_attach(const char *arg, char *mountpoint, char *fstype, char *source)
{
	int err;
	int tmp;
	int fd;

	fd = -1;

	err = snprintf(mountpoint, WATCHMAN_PATH_MAX_LEN, "%s", arg);
	if (unlikely((err < 0) || (err >= WATCHMAN_PATH_MAX_LEN))) {
		WATCHMAN_ERROR("snprintf() failed: mountpoint truncated");
		return -1;
	}

	fd = ::open("/proc/self/mountinfo", O_RDONLY);
	if (unlikely(fd < 0)) {
		WATCHMAN_ERROR("open() failed with errno %d: %s", errno, strerror(errno));
		mountpoint[0] = 0;
		return -errno;
	}

	err = _copy_from_mountinfo(fd, mountpoint, fstype, source);

	tmp = ::close(fd);
	if (unlikely(tmp < 0)) {
		WATCHMAN_ERROR("close() failed with errno %d: %s", errno, strerror(errno));
	}

	if (unlikely(err)) {
		mountpoint[0] = 0;
		fstype[0] = 0;
		source[0] = 0;

		return err;
	}

	WATCHMAN_DEBUG("mountpoint = \"%s\", fstype = \"%s\", mount source = \"%s\"", \
	               mountpoint, fstype, source);

	return 0;
}

bool _cling_file_filesystem_is_mounted(const char *mountpoint, const char *fstype, const char *source)
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

	err = _seek_to_string(fd, mountpoint);
	if (unlikely(err < 0)) {
		mounted = false;
		goto exit;
	}
	
	err = _seek_to_string(fd, "- ");
	if (unlikely(err < 0)) {
		mounted = false;
		goto exit;
	}

	err = _compare_content(fd, fstype, WATCHMAN_FILESYSTEM_MAX_NAME_LEN);
	if (unlikely(err)) {
		mounted = false;
		goto exit;
	}

	err = _skip_whitespace(fd);
	if (unlikely(err)) {
		mounted = false;
		goto exit;
	}

	err = _compare_content(fd, source, WATCHMAN_PATH_MAX_LEN);
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

