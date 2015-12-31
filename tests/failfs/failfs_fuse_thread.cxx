
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>

#include "compiler.h"
#include "failfs_fuse_thread.hxx"
#include "failfs_fuse.h"
#include "failfs.hxx"
#include "error.hxx"


Failfs_Fuse_Thread::Failfs_Fuse_Thread()
: _fs(NULL)
{
	memset(_mountpoint, 0, FAILFS_PATH_MAXLEN);
}

static void *_fuse_main_thread(void *arg)
{
	Failfs *fs = (Failfs *)arg;
	int err;

	err = failfs_fuse_loop();

	fs->send_exit_notification();

	return (void *)(unsigned long long)err;
}

int Failfs_Fuse_Thread::_create_pthread()
{
	int err;

	/* According to man pthread_attr_init(3) this function
	 * always succeeds.
	 */
	pthread_attr_init(&_attr);

	err = pthread_create(&_handle,
	                     NULL,
	                     _fuse_main_thread,
	                     (void *)_fs);
	if (unlikely(err)) {
		FAILFS_ERROR("pthread_create() failed with error %d: %s", err, strerror(err));
		return -err;
	}

	return 0;
}

int Failfs_Fuse_Thread::_cancel_pthread()
{
	int err;

	err = pthread_cancel(_handle);
	if (unlikely(err)) {
		FAILFS_ERROR("pthread_cancel() failed with error %d: %s", err, strerror(err));
		return -err;
	}

	return 0;
}

int Failfs_Fuse_Thread::_join_pthread()
{
	int err;

	err = pthread_join(_handle, NULL);
	if (unlikely(err)) {
		FAILFS_ERROR("pthread_join() failed with error %d: %s", err, strerror(err));
		return -err;
	}

	/* According to man pthread_attr_init(3) this function
	 * always succeeds.
	 */
	pthread_attr_destroy(&_attr);

	return 0;
}

int Failfs_Fuse_Thread::init(const char *mountpoint, Failfs *fs)
{
	int err;

	_fs = fs;

	err = snprintf(_mountpoint, FAILFS_PATH_MAXLEN, "%s", mountpoint);
	if (unlikely((err < 0) || (err >= FAILFS_PATH_MAXLEN))) {
		FAILFS_ERROR("snprintf() failed: mountpoint truncated");
	}

	return mount();
}

int Failfs_Fuse_Thread::fini()
{
	int err;
	int tmp;

	err = 0;

	/* To be safe in case the process has been canceled.
	 */
	tmp = _cancel_pthread();
	if (unlikely(tmp)) {
		err = tmp;
	}
	tmp = _join_pthread();
	if (unlikely(tmp)) {
		err = tmp;
	}

	err = failfs_fuse_fini();
	if (unlikely(err)) {
		FAILFS_ERROR("failfs_fuse_fini() failed with error %d", err);
	}

	return err;
}

int Failfs_Fuse_Thread::mount()
{
	int err;

	err = failfs_fuse_init(_mountpoint, (void *)_fs);
	if (unlikely(err)) {
		FAILFS_ERROR("failfs_fuse_init() failed with error %d", err);
		return err;
	}

	err = _create_pthread();
	if (unlikely(err)) {
		return err;
	}

	return 0;
}

int Failfs_Fuse_Thread::umount()
{
	return fini();
}

int Failfs_Fuse_Thread::remount()
{
	int err;

	err = umount();
	if (unlikely(err))
		return err;

	err = mount();
	if (unlikely(err))
		return err;

	return 0;
}

