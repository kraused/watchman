
#include <string.h>
#include <stdio.h>

#include "error.hxx"

void Error::error(const char *file, const char *func, long line, const char *fmt, ...)
{
	va_list vl;

	va_start(vl, fmt);
	Error::_report("error: ", file, func, line, fmt, vl);
	va_end(vl);
}

void Error::warn(const char *file, const char *func, long line, const char *fmt, ...)
{
	va_list vl;

	va_start(vl, fmt);
	Error::_report("warning: ", file, func, line, fmt, vl);
	va_end(vl);
}

void Error::log(const char *file, const char *func, long line, const char *fmt, ...)
{
	va_list vl;

	va_start(vl, fmt);
	Error::_report("", file, func, line, fmt, vl);
	va_end(vl);
}

void Error::debug(const char *file, const char *func, long line, const char *fmt, ...)
{
	va_list vl;

	va_start(vl, fmt);
	Error::_report("debug: ", file, func, line, fmt, vl);
	va_end(vl);
}

char Error::_buf[4096];

void Error::_report(const char* prefix, const char* file, const char* func, long line,
                    const char* fmt, va_list vl)
{
        vsnprintf(Error::_buf, sizeof(Error::_buf), fmt, vl);

	fprintf(stderr, "<(%s(), %s:%ld> %s%s\n", func, file, line, prefix, Error::_buf);
	fflush (stderr);
}

