
#ifndef WATCHMAN_CLINGY_NAMED_FILE_HXX_INCLUDED
#define WATCHMAN_CLINGY_NAMED_FILE_HXX_INCLUDED 1

#include "config.hxx"
#include "named_file.hxx"

/* File with an associated path on a filesystem. In contrast to Named_File instances
 * of Named_Clingy_File are associated with a particular filesystem and mountpoint and
 * will not allow reopening if that filesystem is not mounted where it is expected.
 */
class Named_Clingy_File : public Named_File
{

public:
			/* Note: The file cannot be used before open() has been
			 *       called.
			 */
			explicit Named_Clingy_File();

public:
			/* Attach the instance to a filesystem mounted on the specified
			 * mountpoint.
			 */
	int		attach(const char *mountpoint);

private:
	int		_copy_from_mountinfo(int fd);

protected:
	bool		can_reopen();

private:
#undef  WATCHMAN_FILESYSTEM_MAX_NAME_LEN
#define WATCHMAN_FILESYSTEM_MAX_NAME_LEN	64

	char		_mountpoint[WATCHMAN_PATH_MAX_LEN];
	char		_fstype[WATCHMAN_FILESYSTEM_MAX_NAME_LEN];
	char		_source[WATCHMAN_PATH_MAX_LEN];

private:
	bool		_filesystem_is_mounted();

};

#endif

