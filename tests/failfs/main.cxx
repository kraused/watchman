
#include <stdlib.h>
#include <pthread.h>

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

	err = pthread_attr_init(&thr->attr);

	err = pthread_create(&thr->handle,
	                     NULL,
	                     _fuse_main_thread,
	                     (void *)thr);

	return 0;
}

static int _cancel_fuse_thread(_Thread *thr)
{
	int err;

	err = pthread_cancel(thr->handle);

	return err;
}

static int _join_fuse_thread(_Thread *thr)
{
	int err;

	err = pthread_join(thr->handle, NULL);

	err = pthread_attr_destroy(&thr->attr);

	return 0;
}

static int _initialize(_Thread *thr, Failfs *fs)
{
	int err;

	err = fs->init_signal_handling();

	err = fs->create_socketpair();
	err = fs->create_mirrordir();

	/* FIXME: Make the mountpoint variable.
	 */
	err = failfs_fuse_init("/dev/shm/failfs", (void *)fs);

	err = _create_fuse_thread(thr);

	/* Bind the socket at the latest possible time.
	 */
	err = fs->bind_listenfd("/dev/shm/failfs.sock");

	return err;
}

static int _finalize(_Thread *thr, Failfs *fs)
{
	int err;

	/* Close the socket at the earliest possible time.
	 */
	err = fs->close_listenfd();

	/* To be safe in case the process has been canceled.
	 */
	err = _cancel_fuse_thread(thr);

	err = failfs_fuse_fini();

	err = _join_fuse_thread(thr);

	err = fs->fini_signal_handling();
	err = fs->remove_mirrordir();
	err = fs->close_socketpair();

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

	err = _initialize(&thr, &fs);
	if (unlikely(err))
		goto exit;

	err = fs.loop();

exit:
	_finalize(&thr, &fs);

	return err;
}

