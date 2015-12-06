
#ifndef WATCHMAN_CHILD_HXX_INCLUDED
#define WATCHMAN_CHILD_HXX_INCLUDED 1

#include "file.hxx"

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

	inline File		*file_o();
	inline File		*file_e();

public:
				/* Soft termination request. On Linux this
				 * is implemented via SIGTERM.
				 */
	virtual int		terminate();
				/* Hard termination.
				 */
	virtual int		kill();
				/* waitpid() wrapper.
				 */
	virtual int		wait();

private:
	int			_send_signal_to_child(int signum);

protected:
	long long		_pid;
	File			_fo;
	File			_fe;
};

inline long long Child::pid() const
{
	return _pid;
}

inline File *Child::file_o()
{
	return &_fo;
}

inline File *Child::file_e()
{
	return &_fe;
}

#endif

