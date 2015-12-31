
#ifndef WATCHMAN_NAMED_FILE_HXX_INCLUDED
#define WATCHMAN_NAMED_FILE_HXX_INCLUDED 1

#include "config.hxx"
#include "file.hxx"

/* File with an associated path on the filesystem. Instances of Named_File can be reopened.
 */
class Named_File : public File
{

public:
			/* Note: The file cannot be used before open() has been
			 *       called.
			 */
			explicit Named_File();

public:
	int		open(const char *path, int oflags, int perms);

protected:
	virtual int	reopen();

private:
	char		_path[WATCHMAN_PATH_MAX_LEN];
	int		_oflags;
	int		_perms;

};

#endif

