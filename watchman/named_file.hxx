
#ifndef WATCHMAN_NAMED_FILE_HXX_INCLUDED
#define WATCHMAN_NAMED_FILE_HXX_INCLUDED 1

#include "watchman/config.hxx"
#include "watchman/file.hxx"

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
	int			open(const char *path, int oflags, int perms);

public:
	virtual const char*	path() const;

public:
	virtual long long	size() const;

protected:
	virtual void		increment_file_size(long long n);

public:
	virtual bool		supports_reopen() const;
	virtual bool		can_reopen();
	virtual int		reopen(const char *path);

public:
	virtual bool		supports_rename() const;
	virtual bool		can_rename();
	virtual int		rename(const char *newpath);

private:
	char			_path[WATCHMAN_PATH_MAX_LEN];
	long long		_size;
	int			_oflags;
	int			_perms;

};

#endif

