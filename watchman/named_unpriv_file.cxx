
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "named_unpriv_file.hxx"
#include "compiler.hxx"
#include "error.hxx"


template<bool stop_on_error>
int _set_effective_ids(uid_t uid, gid_t gid)
{
	int err;

	if (geteuid() != uid) {
		err = seteuid(uid);
		if (unlikely(err < 0)) {
			WATCHMAN_ERROR("seteuid() failed with errno %d: %s", errno, strerror(errno));
			if (stop_on_error) {
				return -1;
			}
		}
	}
	if (getegid() != gid) {
		err = setegid(gid);
		if (unlikely(err < 0)) {
			WATCHMAN_ERROR("setegid() failed with errno %d: %s", errno, strerror(errno));
			if (stop_on_error) {
				return -1;
			}
		}
	}	

	return 0;
}


class RAII_Effective_Ids
{
public:
		explicit RAII_Effective_Ids(int uid, int gid);
		~RAII_Effective_Ids();

private:
	int	_uid;
	int	_gid;
};

RAII_Effective_Ids::RAII_Effective_Ids(int uid, int gid)
: _uid(uid), _gid(gid)
{
}

RAII_Effective_Ids::~RAII_Effective_Ids()
{
	_set_effective_ids<false>(_uid, _gid);
}


Named_Unpriv_File::Named_Unpriv_File(int uid, int gid)
: Named_File(), _uid(uid), _gid(gid)
{
}

int Named_Unpriv_File::open(const char *path, int oflags, int perms)
{
	int err;
	RAII_Effective_Ids _restore_ids(geteuid(), getegid());

	err = _set_effective_ids<true>(_uid, _gid);
	if (unlikely(err < 0)) {
		return err;
	}

	return Named_File::open(path, oflags, perms);
}

int Named_Unpriv_File::reopen()
{
	int err;
	RAII_Effective_Ids _restore_ids(geteuid(), getegid());

	err = _set_effective_ids<true>(_uid, _gid);
	if (unlikely(err < 0)) {
		return err;
	}

	return Named_File::reopen();
}

