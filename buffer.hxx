
#ifndef WATCHMAN_BUFFER_HXX_INCLUDED
#define WATCHMAN_BUFFER_HXX_INCLUDED 1

#include "config.hxx"

struct _Buffer_Line
{
	short			begin;
	short			size;
				/* Warning: No trailing zero.
				 */
	char			line[WATCHMAN_BUFFER_MAX_LINELEN];
};

class _Buffer_Line_Queue
{

public:
				explicit _Buffer_Line_Queue();

public:
				/* Access the head of the queue without
				 * removing it from the queue.
				 */
	_Buffer_Line		*head();
				/* Access the tail of the queue without
				 * removing it from the queue.
				 */
	_Buffer_Line		*tail();

public:
	void			dequeue();
	void			enqueue();

public:
	inline int		length() const;

private:
	_Buffer_Line		_lines[WATCHMAN_BUFFER_QUEUE_LEN];
	int			_head;
	int			_tail;
};

class Buffer
{

public:
				explicit Buffer();

public:
	long long		read_from_stdout(int fd);
	long long		read_from_stderr(int fd);

private:
	long long		_read_from(int fd, _Buffer_Line_Queue *q);

public:
	long long		write_to_stdout(int fd);
	long long		write_to_stderr(int fd);

private:
	long long		_write_to(int fd, _Buffer_Line_Queue *q);

public:
	inline bool		stdout_pending() const;
	inline bool		stderr_pending() const;

	/* TODO Two different queues for stdout and stderr are overkill
	 *      in particular because one usually expects applications
	 *      to write mostly to stdout and rarely to stderr.
	 */
private:
	_Buffer_Line_Queue	_q_o;
	_Buffer_Line_Queue	_q_e;

private:
	char			_buf[WATCHMAN_BUFFER_MAX_LINELEN + 1];

};

inline int _Buffer_Line_Queue::length() const
{
	return _tail - _head;
}

inline bool Buffer::stdout_pending() const
{
	return (_q_o.length() > 0);
}

inline bool Buffer::stderr_pending() const
{
	return (_q_e.length() > 0);
}

#endif

