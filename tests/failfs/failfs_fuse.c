
#define FUSE_USE_VERSION 30
#include <fuse.h>

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "compiler.h"
#include "config.h"
#include "failfs.hxx"

static struct _Fuse_Context {
	struct fuse_chan	*channel;
	struct fuse_args	args;
	struct fuse		*handle;

	char			mountpoint[FAILFS_PATH_MAXLEN];
	void			*failfs;

	char			path[FAILFS_PATH_MAXLEN];

} _context;

static int _send_fuse_cmd_and_recv(int cmd, const char *path)
{
	int err;
	int fuse_err;

	err = Failfs_send_fuse_cmd_and_recv(_context.failfs,
	                                    cmd, path,
	                                    _context.path,
	                                    &fuse_err);
	if (unlikely(err))
		return err;

	if (unlikely(fuse_err))
		return fuse_err;

	return 0;
}

static int _failfs_getattr(const char *path, struct stat *buf)
{
	int err;
	int errsv;

	err = _send_fuse_cmd_and_recv(FAILFS_FUSE_CMD_GETATTR, path);
	if (unlikely(err))
		return err;

	err   = stat(_context.path, buf);
	errsv = errno;

	if (unlikely(err < 0))
		return -errsv;

	return 0;
}

static int _failfs_readdir(const char *path, void *buf,
                           fuse_fill_dir_t filler,
                           off_t offset,
                           struct fuse_file_info *fi)
{
	int err;
	int errsv;
	DIR *d;
	struct dirent *x;

	err = _send_fuse_cmd_and_recv(FAILFS_FUSE_CMD_READDIR, path);
	if (unlikely(err))
		return err;

	d     = opendir(_context.path);
	errsv = errno;

	if (unlikely(!d))
		return -errsv;

	while (NULL != (x = readdir(d))) {
		filler(buf, x->d_name, NULL, 0);
	}

	closedir(d);

	return 0;
}

static int _failfs_access(const char *path, int mode)
{
	int err;
	int errsv;

	err = _send_fuse_cmd_and_recv(FAILFS_FUSE_CMD_ACCESS, path);
	if (unlikely(err))
		return err;

	err   = access(_context.path, mode);
	errsv = errno;

	if (unlikely(err < 0))
		return -errsv;

	return 0;
}

static int _failfs_create(const char *path, mode_t mode,
                          struct fuse_file_info *fi)
{
	int err;
	int errsv;

	err = _send_fuse_cmd_and_recv(FAILFS_FUSE_CMD_CREATE, path);
	if (unlikely(err))
		return err;

	fi->fh = open(_context.path, O_CREAT | O_WRONLY | O_TRUNC, mode);
	errsv  = errno;

	if (unlikely(-1 == fi->fh))
		return -errsv;

	return 0;
}

static int _failfs_open(const char *path,
                        struct fuse_file_info *fi)
{
	int err;
	int errsv;

	err = _send_fuse_cmd_and_recv(FAILFS_FUSE_CMD_OPEN, path);
	if (unlikely(err))
		return err;

	fi->fh = open(_context.path, fi->flags);
	errsv  = errno;

	if (unlikely(-1 == fi->fh))
		return -errsv;

	return 0;
}

static int _failfs_release(const char *path,
                           struct fuse_file_info *fi)
{
	int err;

	err = _send_fuse_cmd_and_recv(FAILFS_FUSE_CMD_RELEASE, path);
	if (unlikely(err))
		return err;

	if (likely(-1 != fi->fh))
		close(fi->fh);

	return 0;
}

static int _failfs_read(const char *path, char *buf,
                        size_t size, off_t offset,
                        struct fuse_file_info *fi)
{
	int err;
	int errsv;

	err = _send_fuse_cmd_and_recv(FAILFS_FUSE_CMD_READ, path);
	if (unlikely(err))
		return err;

	if (unlikely(-1 == fi->fh))
		return -EBADF;

	err   = lseek(fi->fh, offset, SEEK_SET);
	errsv = errno;

	if (unlikely(err < 0))
		return -errsv;

	err   = read(fi->fh, buf, size);
	errsv = errno;

	if (unlikely(err < 0))
		return -errsv;

	/* Return number of bytes read. */
	return err;
}

static int _failfs_write(const char *path, const char *buf,
                         size_t size, off_t offset,
                         struct fuse_file_info *fi)
{
	int err;
	int errsv;

	err = _send_fuse_cmd_and_recv(FAILFS_FUSE_CMD_WRITE, path);
	if (unlikely(err))
		return err;

	if (unlikely(-1 == fi->fh))
		return -EBADF;

	err   = lseek(fi->fh, offset, SEEK_SET);
	errsv = errno;

	if (unlikely(err < 0))
		return -errsv;

	err   = write(fi->fh, buf, size);
	errsv = errno;

	if (unlikely(err < 0))
		return -errsv;

	/* Return number of bytes written. */
	return err;
}

static int _failfs_unlink(const char *path)
{
	int err;
	int errsv;

	err = _send_fuse_cmd_and_recv(FAILFS_FUSE_CMD_UNLINK, path);
	if (unlikely(err))
		return err;

	err   = unlink(_context.path);
	errsv = errno;

	if (unlikely(err < 0))
		return -errsv;

	return 0;
}

/* Required or else a simple "echo 1 > file" will not work.
 * Apparently, open(..., O_TRUNC) is implemented via open(..., )
 * and truncate().
 */
static int _failfs_truncate(const char *path, off_t length)
{
	int err;
	int errsv;

	err = _send_fuse_cmd_and_recv(FAILFS_FUSE_CMD_TRUNCATE, path);
	if (unlikely(err))
		return err;

	err   = truncate(_context.path, length);
	errsv = errno;

	if (unlikely(err < 0))
		return -errsv;

	return 0;
}

static int _failfs_ftruncate(const char *path, off_t length,
                             struct fuse_file_info *fi)
{
	int err;
	int errsv;

	err = _send_fuse_cmd_and_recv(FAILFS_FUSE_CMD_FTRUNCATE, path);
	if (unlikely(err))
		return err;

	if (unlikely(-1 == fi->fh))
		return -EBADF;

	err   = ftruncate(fi->fh, length);
	errsv = errno;

	if (unlikely(err < 0))
		return -errsv;

	return 0;
}

/* FUSE provides two APIs: A low- and a high-level API. The high-level API
 * works with paths whereas the low-level API works solely with inodes. For
 * the kind of filesystem (which mirrors another folder in a different FS)
 * the high-level API is more suitable.
 */

static struct fuse_operations _failfs_ops = {
	.getattr   = _failfs_getattr,
	.readdir   = _failfs_readdir,
	.access    = _failfs_access,
	.create    = _failfs_create,
	.open      = _failfs_open,
	.release   = _failfs_release,
	.read      = _failfs_read,
        .write     = _failfs_write,
	.unlink    = _failfs_unlink,
	.truncate  = _failfs_truncate,
	.ftruncate = _failfs_ftruncate
};

int failfs_fuse_init(const char *mountpoint, void *failfs)
{
	int err;

	err = snprintf(_context.mountpoint, FAILFS_PATH_MAXLEN, "%s", mountpoint);
	if (unlikely((err < 0) || (err >= FAILFS_PATH_MAXLEN)))
		return -1;

	_context.failfs = failfs;

	_context.channel = fuse_mount(_context.mountpoint, &_context.args);
	if (unlikely(!_context.channel)) {
		return -1;
	}

	_context.handle = fuse_new(_context.channel, &_context.args,
	                           &_failfs_ops, sizeof(_failfs_ops), NULL);
	if (unlikely(!_context.handle)) {
		return -1;
	}

	return 0;
}

int failfs_fuse_fini()
{
	if (likely(_context.handle))
		fuse_exit(_context.handle);

	if (likely(_context.channel))
		fuse_unmount(_context.mountpoint, _context.channel);

	return 0;
}

int failfs_fuse_loop()
{
	return fuse_loop(_context.handle);
}

