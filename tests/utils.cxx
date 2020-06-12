
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <pwd.h>

#include "watchman/compiler.hxx"
#include "watchman/error.hxx"

#include "tests/utils.hxx"

int get_uid_and_gid(char *name, int *uid, int *gid)
{
	struct passwd *pwd;

	*uid = 0;
	*gid = 0;

	pwd = getpwnam(name);
	if (unlikely(!pwd)) 
	{
		WATCHMAN_ERROR("getpwnam() failed with errno %d: %s", errno, strerror(errno));
		return -errno;
	}

	*uid = pwd->pw_uid;
	*gid = pwd->pw_gid;

	return 0;
}

