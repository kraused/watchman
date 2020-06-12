
#ifndef WATCHMAN_CLINGY_FILE_HXX_INCLUDED
#define WATCHMAN_CLINGY_FILE_HXX_INCLUDED 1

#include "watchman/config.hxx"
#include "watchman/named_file.hxx"

/* File with an associated path on a filesystem. In contrast to File_Type instances
 * of Clingy_File are associated with a particular filesystem and mountpoint and
 * will not allow reopening if that filesystem is not mounted where it is expected.
 */
template<class File_Type>
class Clingy_File : public File_Type
{

public:
			/* Note: The file cannot be used before open() has been
			 *       called.
			 */
			explicit Clingy_File();

public:
			/* Attach the instance to a filesystem mounted on the specified
			 * mountpoint.
			 */
	int		attach(const char *mountpoint);

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


void _clingy_file_clean(char *mountpoint, char *fstype, char *source);
bool _clingy_file_attach(const char *arg, char *mountpoint, char *fstype, char *source);
bool _clingy_file_filesystem_is_mounted(const char *mountpoint, const char *fstype, const char *source);

template<class File_Type>
Clingy_File<File_Type>::Clingy_File()
{
	_clingy_file_clean(_mountpoint, _fstype, _source);
}

template<class File_Type>
int Clingy_File<File_Type>::attach(const char *mountpoint)
{
	return _clingy_file_attach(mountpoint, _mountpoint, _fstype, _source);
}

template<class File_Type>
bool Clingy_File<File_Type>::can_reopen()
{
	bool mounted;

	if (0 == _mountpoint[0])	/* attach() failed or has not been called. */
		return false;

	mounted = _filesystem_is_mounted();

	return mounted;
}

template<class File_Type>
bool Clingy_File<File_Type>::_filesystem_is_mounted()
{
	return _clingy_file_filesystem_is_mounted(_mountpoint, _fstype, _source);
}

#endif
