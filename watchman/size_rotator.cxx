
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>

#include "size_rotator.hxx"
#include "compiler.hxx"
#include "error.hxx"
#include "file.hxx"


Size_Rotator::Size_Rotator(long long threshold)
: _threshold(threshold)
{
	memset(_newpath, 0, WATCHMAN_PATH_MAX_LEN);
}

bool Size_Rotator::file_requires_rotation(File *f)
{
	return (f->size() > _threshold);
}

const char* Size_Rotator::transform_file_path(File *f)
{
	int err;
	char tmp[64];
	struct timeval tv;
	struct tm* ti;

	err = gettimeofday(&tv, nullptr);
	if (unlikely(err < 0)) {
		WATCHMAN_ERROR("gettimeofday() failed with errno %d: %s", errno, strerror(errno));
		return nullptr;
	}
	ti = localtime(&tv.tv_sec);
	if (unlikely(!ti)) {
		WATCHMAN_ERROR("localtime() failed");
		return nullptr;
	}

	err = strftime(tmp, sizeof(tmp), "%Y%m%dT%H%M%S", ti);
	if (unlikely(0 == err)) {
		WATCHMAN_ERROR("strftime() failed");
		return nullptr;
	}

	err = snprintf(_newpath, WATCHMAN_PATH_MAX_LEN, "%s-%s", f->path(), tmp);
	if (unlikely((err < 0) || (err >= WATCHMAN_PATH_MAX_LEN))) {
		WATCHMAN_ERROR("snprintf() failed: path truncated");
		return nullptr;
        }

	return _newpath;
}
