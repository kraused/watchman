
#include <errno.h>

#include "compiler.h"
#include "failfs.hxx"
#include "error.hxx"

static int _initialize(Failfs *fs, const char *mountpoint)
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

	err = fs->create_fuse_thread(mountpoint);
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

static int _finalize(Failfs *fs)
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

	tmp = fs->destroy_fuse_thread();
	if (unlikely(tmp)) {
		err = tmp;
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
	int err;

	if (!argv[1]) {
		FAILFS_ERROR("usage: failfs.exe <mountpoint>");
		return -EINVAL;
	}

	err = _initialize(&fs, argv[1]);
	if (unlikely(err))
		goto exit;

	err = fs.loop();

exit:
	_finalize(&fs);

	return err;
}

