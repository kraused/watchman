
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>

#include "compiler.h"
#include "config.h"
#include "error.hxx"
#include "failfs.hxx"

int main(int argc, char **argv)
{
	int sock;
	int err;
	int cmd;
	struct sockaddr_un addr;

	sock = socket(AF_UNIX, SOCK_STREAM, 0);
	if (unlikely(sock < 0)) {
		FAILFS_ERROR("socket() failed with errno %d: %s", errno, strerror(errno));
		return -errno;
	}

	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;

	err = snprintf(addr.sun_path, sizeof(addr.sun_path), FAILFS_CMD_SOCKET);
	if (unlikely((err < 0) || (err >= FAILFS_PATH_MAXLEN))) {
		FAILFS_ERROR("snprintf() failed: unix domain socket path truncated");
		return -ENAMETOOLONG;
	}

	err = connect(sock, (const struct sockaddr *)&addr, sizeof(addr));
	if (unlikely(err < 0)) {
		FAILFS_ERROR("connect() failed with errno %d: %s", errno, strerror(errno));
		return -errno;
	}

	cmd = FAILFS_CMD_NOOP;

	if (argv[1]) {
		if (!strcmp("stale", argv[1])) {
			cmd = FAILFS_CMD_CHANGE_STATE_STALE;
		}
		if (!strcmp("normal", argv[1])) {
			cmd = FAILFS_CMD_CHANGE_STATE_NORMAL;
		}
		if (!strcmp("remount", argv[1])) {
			cmd = FAILFS_CMD_REMOUNT;
		}
		if (!strcmp("umount", argv[1])) {
			cmd = FAILFS_CMD_UMOUNT;
		}
		if (!strcmp("mount", argv[1])) {
			cmd = FAILFS_CMD_MOUNT;
		}
		if (!strcmp("exit", argv[1])) {
			cmd = FAILFS_CMD_EXIT;
		}
	}

	err = write(sock, &cmd, sizeof(int));
	if (unlikely(err != (int )sizeof(int))) {
		FAILFS_ERROR("write() failed with errno %d: %s", errno, strerror(errno));
		return -errno;
	}

	err = close(sock);
	if (unlikely(err < 0)) {
		FAILFS_ERROR("close() failed with errno %d: %s", errno, strerror(errno));
		return -errno;
	}

	return 0;
}

