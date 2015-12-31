
#ifndef WATCHMAN_FILE_HXX_INCLUDED
#define WATCHMAN_FILE_HXX_INCLUDED 1

enum
{
	WATCHMAN_FILE_STATE_CLOSED,
	WATCHMAN_FILE_STATE_HEALTHY,
	WATCHMAN_FILE_STATE_UNHEALTHY,
	WATCHMAN_FILE_STATE_STALE
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
			/* Function called to reopen the file if
			 * an error, such as a stale file handle,
			 * is detected.
			 * The function is only used if _can_reopen
			 * is true.
			 */
	virtual int	reopen();

public:
	bool		can_reopen() const;

protected:
	bool		_can_reopen;

protected:
	int		_last_write_failed;

};

inline int File::fileno() const
{
	return _fd;
}

inline int File::state() const
{
	return _state;
}

inline bool File::can_reopen() const
{
	return _can_reopen;
}

#endif

