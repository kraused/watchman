
#ifndef WATCHMAN_FILE_HXX_INCLUDED
#define WATCHMAN_FILE_HXX_INCLUDED 1

class File
{

public:
			explicit File(int fd);

public:
	long long	read(void *buf, long long nbyte);
	long long	write(const void *buf, long long nbyte);

protected:
			/* Function called to reopen the file if
			 * an error, such as a stale file handle,
			 * is detected.
			 * The function is only used if _can_reopen
			 * is true.
			 */
	virtual int	reopen();

protected:
	int		_fd;

public:
			/* Get the file descriptor. */
	inline int	fileno() const;

protected:
	bool		_can_reopen;

protected:
	long long	_read_err;
	long long	_write_err;

};

inline int File::fileno() const
{
	return _fd;
}

#endif

