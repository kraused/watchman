
#define _GNU_SOURCE

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

FILE *_logfile;

static char *_translate_path(const char *path)
{
	char *q;
	asprintf(&q, "%s%s", "/dev/shm/failfs", path);

	return q;
}

static int _failfs_getattr(const char *path, struct stat *buf)
{
	char *q = _translate_path(path);
	int errsv;
	int x;

	x     = stat(q, buf);
	errsv = errno;
	
	free(q);

	fprintf(_logfile, "getattr(\"%s\") = %d (%d)\n", path, x, errsv);
	fflush (_logfile);

	if (x < 0)
		return -errsv;

	return 0;
}

static int _failfs_readdir(const char *path, void *buf,
                           fuse_fill_dir_t filler,
                           off_t offset,
                           struct fuse_file_info *fi)
{
	char *q = _translate_path(path);
	DIR  *d;
	struct dirent *x;

	d = opendir(q);

	while (NULL != (x = readdir(d))) {
		filler(buf, x->d_name, NULL, 0);
	}

	closedir(d);
	free(q);

	return 0;
}

static int _failfs_access(const char *path, int mode)
{
	char *q = _translate_path(path);
	int errsv;
	int x;

	x     = access(path, mode);
	errsv = errno;

	free(q);
	
	fprintf(_logfile, "access(\"%s\") = %d (%d)\n", path, x, errsv);
	fflush (_logfile);

	if (x < 0)
		return -errsv;

	return 0;
}

static int _failfs_create(const char *path, mode_t mode,
                          struct fuse_file_info *fi)
{
	char *q = _translate_path(path);
	int errsv;

	fi->fh = open(q, O_CREAT | O_WRONLY | O_TRUNC, mode);
	errsv  = errno;

	free(q);

	fprintf(_logfile, "create(\"%s\") = %d (%d)\n", path, (int )fi->fh, errsv);
	fflush (_logfile);

	if (-1 == fi->fh)
		return -errsv;

	return 0;
}

static int _failfs_open(const char *path,
                        struct fuse_file_info *fi)
{
	char *q = _translate_path(path);
	int errsv;

	fi->fh = open(q, fi->flags);
	errsv  = errno;

	free(q);

	fprintf(_logfile, "open(\"%s\", %d) = %d (%d)\n", path, fi->flags, (int )fi->fh, errsv);
	fflush (_logfile);

	if (-1 == fi->fh)
		return -errsv;

	return 0;
}

static int _failfs_release(const char *path,
                           struct fuse_file_info *fi)
{
	fprintf(_logfile, "release(\"%s\" | %d)\n", path, (int )fi->fh);
	fflush (_logfile);

	if (-1 != fi->fh)
		close(fi->fh);

	return 0;
}

static int _failfs_read(const char *path, char *buf,
                        size_t size, off_t offset,
                        struct fuse_file_info *fi)
{
	int x;

	fprintf(_logfile, "read(\"%s\" | %d)\n", path, (int )fi->fh);
	fflush (_logfile);

	if (-1 == fi->fh)
		return -EBADF;

	x = lseek(fi->fh, offset, SEEK_SET);
	if (-1 == x)
		return -errno;

	x = read(fi->fh, buf, size);
	if (x < 0)
		return -errno;

	return x;
}

static int _failfs_write(const char *path, const char *buf,
                         size_t size, off_t offset,
                         struct fuse_file_info *fi)
{
	int x;

	fprintf(_logfile, "write(\"%s\" | %d)\n", path, (int )fi->fh);
	fflush (_logfile);

	if (-1 == fi->fh)
		return -EBADF;

	x = lseek(fi->fh, offset, SEEK_SET);
	if (-1 == x)
		return -errno;

	x = write(fi->fh, buf, size);
	if (x < 0)
		return -errno;

	return x;
}

static int _failfs_unlink(const char *path)
{
	char *q = _translate_path(path);
	int errsv;
	int x;

	x     = unlink(q);
	errsv = errno;
	
	free(q);

	if (x < 0)
		return -errsv;

	return 0;
}

/* Required or else a simple "echo 1 > file" will not work.
 * Apparently, open(..., O_TRUNC) is implemented via open(..., )
 * and truncate().
 */
static int _failfs_truncate(const char *path, off_t length)
{
	char *q = _translate_path(path);
	int errsv;
	int x;

	x     = truncate(q, length);
	errsv = errno;
	
	free(q);

	fprintf(_logfile, "truncate(\"%s\")\n", path);
	fflush (_logfile);

	if (x < 0)
		return -errsv;

	return 0;
}

static int _failfs_ftruncate(const char *path, off_t length,
                             struct fuse_file_info *fi)
{
	int x;

	fprintf(_logfile, "ftruncate(\"%s\" | %d)\n", path, (int )fi->fh);
	fflush (_logfile);

	if (-1 == fi->fh)
		return -EBADF;

	x = ftruncate(fi->fh, length);
	if (-1 == x)
		return -errno;

	return x;
}

/* FUSE provides two APIs: A low- and a high-level API. The high-level API
 * works with paths whereas the low-level API works solely with inodes. For
 * the kind of filesystem (which mirrors another folder in a different FS)
 * the high-level API is more suitable.
 */

static struct fuse_operations failfs_ops = {
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

int main(int argc, char **argv)
{
	_f = fopen("failfs.log", "w");

	return fuse_main(argc, argv, &failfs_ops, NULL);
}

