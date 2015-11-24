
#ifndef WATCHMAN_FILE_PAIR_HXX_INCLUDED
#define WATCHMAN_FILE_PAIR_HXX_INCLUDED 1

class File_Pair
{

public:
			explicit File_Pair(int fd_o, int fd_e);

public:
	inline int	stdout_fileno() const;
	inline int	stderr_fileno() const;

public:
	void		set_stdout_fileno(int fd_o);
	void		set_stderr_fileno(int fd_e);

private:
	int		_fd_o;
	int		_fd_e;

};

inline int File_Pair::stdout_fileno() const
{
	return _fd_o;
}

inline int File_Pair::stderr_fileno() const
{
	return _fd_e;
}

#endif

