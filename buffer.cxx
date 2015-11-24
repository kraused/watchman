
#include <unistd.h>
#include <errno.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "buffer.hxx"
#include "compiler.hxx"

_Buffer_Line_Queue::_Buffer_Line_Queue()
: _head(0), _tail(0)
{
	int i;

	for (i = 0; i < WATCHMAN_BUFFER_QUEUE_LEN; ++i) {
		_lines[i].begin = 0;
		_lines[i].size  = 0;
		memset(_lines, 0, WATCHMAN_BUFFER_MAX_LINELEN);
	}
}

_Buffer_Line *_Buffer_Line_Queue::head()
{
	if (0 == length())
		return NULL;

	return &_lines[_head % WATCHMAN_BUFFER_QUEUE_LEN];
}

_Buffer_Line *_Buffer_Line_Queue::tail()
{
	if (0 == length())
		return NULL;

	return &_lines[(_tail - 1) % WATCHMAN_BUFFER_QUEUE_LEN];
}

void _Buffer_Line_Queue::dequeue()
{
	++_head;
}

void _Buffer_Line_Queue::enqueue()
{
	++_tail;

	tail()->begin = 0;
	tail()->size  = 0;
}

Buffer::Buffer()
{
}

long long Buffer::read_from_stdout(int fd)
{
	return _read_from(fd, &_q_o);
}

long long Buffer::read_from_stderr(int fd)
{
	return _read_from(fd, &_q_e);
}

long long Buffer::write_to_stdout(int fd)
{
	return _write_to(fd, &_q_o);
}

long long Buffer::write_to_stderr(int fd)
{
	return _write_to(fd, &_q_e);
}

long long Buffer::flush_stdout(int fd)
{
	return _flush(fd, &_q_o);
}

long long Buffer::flush_stderr(int fd)
{
	return _flush(fd, &_q_e);
}

long long Buffer::_read_from(int fd, _Buffer_Line_Queue *q)
{
	long long n;
	int i;
	char *x;
	_Buffer_Line *line = q->tail();

	if (!line) {
		q->enqueue();
		line = q->tail();
	}

	/* FIXME Handle errors.
	 */

	n = read(fd, _buf, WATCHMAN_BUFFER_MAX_LINELEN);

	if (likely(n >= 0))
		_buf[n] = 0;

	for (x = _buf; *x; ++x) {
		i = line->begin + line->size;

		if (i >= WATCHMAN_BUFFER_MAX_LINELEN) {
			q->enqueue();
			line = q->tail();
		}

		line->line[i] = *x;
		++line->size;

		if('\n' == *x) {
			q->enqueue();
			line = q->tail();
		}
	}

	return n;
}

long long Buffer::_write_to(int fd, _Buffer_Line_Queue *q)
{
	long long n;
	_Buffer_Line *line = q->head();

	/* FIXME Handle errors.
	 */

	n = 0;
	if (line->size > 0)
		n = write(fd, line->line + line->begin, line->size);

	if (likely(n > 0)) {
		line->begin += n;
		line->size  -= n;
	}

	if (0 == line->size)
		q->dequeue();

	return n;
}

long long Buffer::_flush(int fd, _Buffer_Line_Queue *q)
{
	long long n, x;

	n = 0;

	/* FIXME Handle errors and make sure that this loop is not an infinite
	 *       one.
	 */
	while (q->length() > 0) {
		x = _write_to(fd, q);
		if (x > 0)
			n += x;
	}

	return n;
}

