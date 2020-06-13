
#include <unistd.h>
#include <errno.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "buffer.hxx"
#include "compiler.hxx"
#include "file.hxx"

static long long _find_last_newline(const char *start, const char *end);
static void _copy_to_Buffer_Line(const char *start, const char *end, _Buffer_Line **line, _Buffer_Line_Queue *q);

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
		return nullptr;

	return &_lines[_head % WATCHMAN_BUFFER_QUEUE_LEN];
}

_Buffer_Line *_Buffer_Line_Queue::tail()
{
	if (0 == length())
		return nullptr;

	return &_lines[(_tail - 1) % WATCHMAN_BUFFER_QUEUE_LEN];
}

void _Buffer_Line_Queue::dequeue()
{
	head()->begin = 0;
	head()->size  = 0;

	++_head;
}

void _Buffer_Line_Queue::enqueue()
{
	++_tail;

	if (length() > WATCHMAN_BUFFER_QUEUE_LEN)
		++_head;

	tail()->begin = 0;
	tail()->size  = 0;
}

Buffer::Buffer()
{
}

long long Buffer::read_from_stdout(File *f)
{
	return _read_from(f, &_q_o);
}

long long Buffer::read_from_stderr(File *f)
{
	return _read_from(f, &_q_e);
}

long long Buffer::write_to_stdout(File *f)
{
	return _write_to(f, &_q_o);
}

long long Buffer::write_to_stderr(File *f)
{
	return _write_to(f, &_q_e);
}

long long Buffer::_read_from(File *f, _Buffer_Line_Queue *q)
{
	long long n, k;
	_Buffer_Line *line = q->tail();	/* May be nullptr. */

#if 0
	/* FIXME With this change, test4 will succeed but we will be 
	 * stalling the child processes in case the output file cannot
	 * be written to for a longer time.
	 */
	/* Avoid the need to drop any read content if we overrun the
	 * buffer space.
	 */
	if (q->length() >= (WATCHMAN_BUFFER_QUEUE_LEN - 1)) {
		return 0;
	}
#endif

	n = f->read(_buf, WATCHMAN_BUFFER_MAX_LINELEN);
	if (unlikely(n < 0)) {
		return n;
	}

	/* Try to fit as many output lines into one _Buffer_Line to avoid wasting
	 * space. */
	k = _find_last_newline(_buf, (_buf + n));
	if (k >= 0) {
		_copy_to_Buffer_Line(_buf, (_buf + k + 1), &line, q);
		line = nullptr;	/* Grep a new line in next call to _copy_to_Buffer_Line(). */
		_copy_to_Buffer_Line((_buf + k + 1), (_buf + n), &line, q);
	} else {
		_copy_to_Buffer_Line(_buf, (_buf + n), &line, q);
	}

	return n;
}

long long Buffer::_write_to(File *f, _Buffer_Line_Queue *q)
{
	long long n;
	_Buffer_Line *line = q->head();

	n = 0;
	if (line->size > 0) {
		n = f->write(line->line + line->begin, line->size);
	}

	if (likely(n > 0)) {
		f->flag_as_dirty();

		line->begin += n;
		line->size  -= n;
	}

	if (0 == line->size) {
		/* The write algorithm itself does not care about the newline.
		 * Therefore there is a chance of having a file marked almost
		 * permanently as dirty and hence blocking any rotation. If this
		 * happens in practice, the right approach would be to mark the
		 * buffer lines as complete/incomplete and only flush complete
		 * lines. A special handling of the flush at the end of the program
		 * execution would be required.
		 */
		if ('\n' == *(line->line + line->begin - 1))
			f->flag_as_clean();

		q->dequeue();
	}

	return n;
}

static long long _find_last_newline(const char *start, const char *end)
{
	long long k = -1;
	const char *x;

	for(x = start; x != end; ++x) {
		if ('\n' == *x)
			k = (x - start);
	}

	return k;
}

static void _copy_to_Buffer_Line(const char *start, const char *end, _Buffer_Line **line, _Buffer_Line_Queue *q)
{
	long long i;
	const char *x;

	if(nullptr == (*line)) {
		q->enqueue();
		*line = q->tail();
	}

	for (x = start; x != end; ++x) {
		i = (*line)->begin + (*line)->size;

		if (i >= WATCHMAN_BUFFER_MAX_LINELEN) {
			q->enqueue();
			*line = q->tail();

			i = (*line)->begin + (*line)->size;
		}

		(*line)->line[i] = *x;
		++(*line)->size;
	}
}

