
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
	long long	read(void *buf, long long nbyte);
	long long	write(const void *buf, long long nbyte);

public:
	int		state() const;

protected:
	int		_state;

public:
			/* Get the file descriptor. */
	inline int	fileno() const;

protected:
	int		_fd;

public:
			/* Returns true if a call to reopen() might be
			 * attempted. can_reopen() might or might not be
			 * pure and results from the call should not be
			 * cached.
			 */
	virtual bool	can_reopen();

			/* Function called to reopen the file if
			 * an error, such as a stale file handle,
			 * is detected.
			 * The function is only used if _can_reopen
			 * is true.
			 */
	virtual int	reopen();

protected:
	int		_last_write_failed;

public:
			/* Force a different file state. This should not be necessary and
			 * probably indicates a bug.
			 */
	void		force_different_state(int state);

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

