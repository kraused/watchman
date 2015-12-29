
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>

#include "compiler.h"
#include "failfs.hxx"
#include "failfs_fuse.h"
#include "error.hxx"

struct _Thread
{
	pthread_t	handle;
	pthread_attr_t	attr;
	struct {
		int	argc;
		char	**argv;
		Failfs	*fs;
	} arg;
};

static void *_fuse_main_thread(void *arg)
{
	_Thread *thr = (_Thread *)arg;
	int err;

	err = failfs_fuse_loop();

	thr->arg.fs->send_exit_notification();

	return (void *)(unsigned long long)err;
}

static int _create_fuse_thread(_Thread *thr)
{
	int err;

	/* According to man pthread_attr_init(3) this function
	 * always succeeds.
	 */
	pthread_attr_init(&thr->attr);

	err = pthread_create(&thr->handle,
	                     NULL,
	                     _fuse_main_thread,
	                     (void *)thr);
	if (unlikely(err)) {
		FAILFS_ERROR("pthread_create() failed with error %d: %s", err, strerror(err));
		return -err;
	}

	return 0;
}

static int _cancel_fuse_thread(_Thread *thr)
{
	int err;

	err = pthread_cancel(thr->handle);
	if (unlikely(err)) {
		FAILFS_ERROR("pthread_cancel() failed with error %d: %s", err, strerror(err));
		return -err;
	}

	return 0;
}

static int _join_fuse_thread(_Thread *thr)
{
	int err;

	err = pthread_join(thr->handle, NULL);
	if (unlikely(err)) {
		FAILFS_ERROR("pthread_join() failed with error %d: %s", err, strerror(err));
		return -err;
	}

	/* According to man pthread_attr_init(3) this function
	 * always succeeds.
	 */
	pthread_attr_destroy(&thr->attr);

	return 0;
}

static int _initialize(_Thread *thr, Failfs *fs, const char *mountpoint)
{
	int err;

	err = fs->init_signal_handling();
	if (unlikely(err)) {
		return err;
	}
	err = fs->create_socketpair();
	if (unlikely(err)) {
		return err;
	}
	err = fs->create_mirrordir();
	if (unlikely(err)) {
		return err;
	}

	err = failfs_fuse_init(mountpoint, (void *)fs);
	if (unlikely(err)) {
		FAILFS_ERROR("failfs_fuse_init() failed with error %d", err);
		return err;
	}

	err = _create_fuse_thread(thr);
	if (unlikely(err)) {
		return err;
	}

	/* Bind the socket at the latest possible time.
	 */
	err = fs->bind_listenfd(FAILFS_CMD_SOCKET);
	if (unlikely(err)) {
		return err;
	}

	return 0;
}

static int _finalize(_Thread *thr, Failfs *fs)
{
	int err;
	int tmp;

	err = 0;

	/* Close the socket at the earliest possible time.
	 */
	tmp = fs->close_listenfd();
	if (unlikely(tmp)) {
		err = tmp;
	}

	/* To be safe in case the process has been canceled.
	 */
	tmp = _cancel_fuse_thread(thr);
	if (unlikely(tmp)) {
		err = tmp;
	}
	tmp = _join_fuse_thread(thr);
	if (unlikely(tmp)) {
		err = tmp;
	}

	err = failfs_fuse_fini();
	if (unlikely(err)) {
		FAILFS_ERROR("failfs_fuse_fini() failed with error %d", err);
	}

	tmp = fs->fini_signal_handling();
	if (unlikely(tmp)) {
		err = tmp;
	}
	tmp = fs->remove_mirrordir();
	if (unlikely(tmp)) {
		err = tmp;
	}
	tmp = fs->close_socketpair();
	if (unlikely(tmp)) {
		err = tmp;
	}

	return err;
}

int main(int argc, char **argv)
{
	Failfs fs;
	_Thread thr;
	int err;

	/* non-trivial designated initializers not supported :( */
	thr.arg.argc = argc;
	thr.arg.argv = argv;
	thr.arg.fs   = &fs;

	if (!argv[1]) {
		FAILFS_ERROR("usage: failfs.exe <mountpoint>");
		return -EINVAL;
	}

	err = _initialize(&thr, &fs, argv[1]);
	if (unlikely(err))
		goto exit;

	err = fs.loop();

exit:
	_finalize(&thr, &fs);

	return err;
}

