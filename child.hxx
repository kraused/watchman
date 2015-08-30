
#ifndef WATCHMAN_CHILD_HXX_INCLUDED
#define WATCHMAN_CHILD_HXX_INCLUDED 1

class Child
{

public:
	virtual int		execute() = 0;

public:
				/* Retrieve the process id of the
				 * child.
				 */
	inline long long	pid() const;

public:
				/* Soft termination request. On Linux this
				 * is implemented via SIGTERM.
				 */
	int			terminate();
				/* Hard termination.
				 */
	int			kill();


private:
	long long		_pid;
	int			_fd_o;
	int			_fd_e;
};

inline long long Child::pid() const
{
	return _pid;
}

#endif

