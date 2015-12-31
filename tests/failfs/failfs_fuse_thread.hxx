
#ifndef FAILFS_FAILFS_FUSE_THREAD_HXX_INCLUDED
#define FAILFS_FAILFS_FUSE_THREAD_HXX_INCLUDED 1

#include <pthread.h>

#include "config.h"

class Failfs;

class Failfs_Fuse_Thread
{

public:
				explicit Failfs_Fuse_Thread();

public:
	int			init(const char *mountpoint, Failfs *fs);
	int			fini();

public:
	int			mount();
	int			umount();
	int			remount();

private:
	int			_create_pthread();
	int			_cancel_pthread();
	int			_join_pthread();

private:
	pthread_t		_handle;
	pthread_attr_t		_attr;

private:
	Failfs			*_fs;
	char			_mountpoint[FAILFS_PATH_MAXLEN];

};

#endif

