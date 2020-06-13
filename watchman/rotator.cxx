
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "rotator.hxx"
#include "compiler.hxx"
#include "error.hxx"
#include "file.hxx"


bool Rotator::file_is_supported(const File *f) const
{
	return (nullptr != f->path()) && f->supports_rename() && f->supports_reopen();
}

bool Rotator::file_can_be_rotated(File *f)
{
	return file_is_supported(f) && f->can_rename() && f->can_reopen();
}

int Rotator::file_rotate(File *f)
{
	int err;
	const char *newpath;

	err = snprintf(_path, WATCHMAN_PATH_MAX_LEN, "%s", f->path());
	if (unlikely((err < 0) || (err >= WATCHMAN_PATH_MAX_LEN))) {
		WATCHMAN_ERROR("snprintf() failed: path truncated");
		return err;
        }

	WATCHMAN_DEBUG("Rotating file %p at \"%s\"", f, _path);

	newpath = transform_file_path(f);
	if (unlikely(!newpath)) {
		return -1;
	}

	WATCHMAN_DEBUG("New path = \"%s\"", newpath);

	err = f->rename(newpath);
	if (unlikely(err)) {
		WATCHMAN_ERROR("rename() returned error %d", err);
		return err;
	}
	err = f->reopen(_path);
	if (unlikely(err)) {
		WATCHMAN_ERROR("reopen() returned error %d", err);
		return err;
	}

	return 0;
}
