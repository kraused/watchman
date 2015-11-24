
#ifndef WATCHMAN_CHILD_HXX_INCLUDED
#define WATCHMAN_CHILD_HXX_INCLUDED 1

#include "file_pair.hxx"

class Child
{

public:
				explicit Child();

public:
	virtual int		execute() = 0;

public:
				/* Retrieve the process id of the
				 * child.
				 */
	inline long long	pid() const;

	inline int		stdout_fileno() const;
	inline int		stderr_fileno() const;

public:
				/* Soft termination request. On Linux this
				 * is implemented via SIGTERM.
				 */
	virtual int		terminate();
				/* Hard termination.
				 */
	virtual int		kill();

private:
	int			_send_signal_to_child(int signum);

protected:
	long long		_pid;
	File_Pair		_fds;
};

inline long long Child::pid() const
{
	return _pid;
}

inline int Child::stdout_fileno() const
{
	return _fds.stdout_fileno();
}

inline int Child::stderr_fileno() const
{
	return _fds.stderr_fileno();
}

#endif

