
#ifndef WATCHMAN_NAMED_UNPRIV_FILE_HXX_INCLUDED
#define WATCHMAN_NAMED_UNPRIV_FILE_HXX_INCLUDED 1

#include "watchman/config.hxx"
#include "watchman/named_file.hxx"

/* File with an associated path on the filesystem and an unprivileged users.
 * Named_Unpriv_File instances are required, e.g., to write to root-squash mounted
 * file systems.
 */
class Named_Unpriv_File : public Named_File
{

public:
			explicit Named_Unpriv_File(int uid, int gid);

public:
	int		open(const char *path, int oflags, int perms);

private:
	int		_drop_privileges();

protected:
	virtual int	reopen(const char *path);

private:
	virtual int	rename(const char *newpath);

private:
	int		_uid;
	int		_gid;
};

#endif

