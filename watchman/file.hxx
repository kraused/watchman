
#ifndef WATCHMAN_FILE_HXX_INCLUDED
#define WATCHMAN_FILE_HXX_INCLUDED 1

enum
{
	WATCHMAN_FILE_STATE_CLOSED	= 1,
	WATCHMAN_FILE_STATE_HEALTHY	= 2,
	WATCHMAN_FILE_STATE_UNHEALTHY	= 3,
	WATCHMAN_FILE_STATE_STALE	= 4
};

class File
{

public:
				explicit File(int fd);

public:
	long long		read(void *buf, long long nbyte);
	long long		write(const void *buf, long long nbyte);

public:
	int			state() const;

protected:
	int			_state;

public:
				/* Get the path of the file. Please note that NULL
				 * is a perfectly valid return value for unnamed files
				 * or stdout/stderr of processes.
				 */
	virtual const char*	path() const;

public:
				/* Get the file size. The size may be < 0 if files do
				 * not have a well-defined size.
				 */
	virtual long long	size() const;

protected:
	virtual void		increment_file_size(long long n);

public:
				/* Get the file descriptor. */
	inline int		fileno() const;

protected:
	int			_fd;

public:
				/* Returns true if the file supports the reopen() function.
				 */
	virtual bool		supports_reopen() const;

				/* Returns true if a call to reopen() might be
				 * attempted. can_reopen() might or might not be
				 * pure and results from the call should not be
				 * cached.
				 * Please note that while supports_reopen() answers the
				 * question whether the file supports reopening at all,
				 * can_reopen() provides an answer whether a reopen() at
				 * the current point could success.
				 */
	virtual bool		can_reopen();

				/* Function called to reopen the file if
				 * an error, such as a stale file handle,
				 * is detected.
				 * The function is only used if _can_reopen
				 * is true.
				 */
	virtual int		reopen(const char *path = nullptr);

public:
				/* Returns true if the file supports the rename() function.
				 * This is used by the rotation logic to understand if the
				 * file can be rotated.
				 */
	virtual bool		supports_rename() const;

				/* Returns true if a call to rename() might be
				 * attempted. The same disclaimer as for can_reopen() applies.
				 */
	virtual bool		can_rename();

				/* Rename the file to newpath without closing it.
				 */
	virtual int		rename(const char *newpath);

public:
				/* Flag the file as dirty or clean. This status is used by the
				 * rotation mechanism to decide whether or not a rotation can
				 * be performed.
				 */
	virtual void		flag_as_dirty();
	virtual void		flag_as_clean();

	virtual bool		is_clean() const;

protected:
	bool			_clean;

protected:
	int			_last_write_failed;

public:
				/* Force a different file state. This should not be necessary and
				 * probably indicates a bug.
				 */
	void			force_different_state(int state);

};

inline int File::fileno() const
{
	return _fd;
}

inline int File::state() const
{
	return _state;
}

#endif

