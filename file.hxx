
#ifndef WATCHMAN_FILE_HXX_INCLUDED
#define WATCHMAN_FILE_HXX_INCLUDED 1

class File
{

public:
			explicit File(int fd);

public:
			/* Get the file descriptor. */
	inline int	fileno() const;

public:
	long long	read(void *buf, long long nbyte);

public:
	long long	write(const void *buf, long long nbyte);	

protected:
			/* Function called to reopen the file if
			 * an error, such as a stale file handle,
			 * is detected.
			 * The function is only used if _can_reopen
			 * is true.
			 */
	virtual int	reopen();

private:
	int		_fd;

protected:
	bool		_can_reopen;

};

inline int File::fileno() const
{
	return _fd;
}

#endif

