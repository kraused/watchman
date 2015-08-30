
#ifndef WATCHMAN_ERROR_HXX_INCLUDED
#define WATCHMAN_ERROR_HXX_INCLUDED

#include <stdarg.h>

class Error
{

public:
	static void	error(const char *file, const char *func, long line, const char *fmt, ...);
	static void	warn (const char *file, const char *func, long line, const char *fmt, ...);
	static void	log  (const char *file, const char *func, long line, const char *fmt, ...);
	static void	debug(const char *file, const char *func, long line, const char *fmt, ...);

private:
	static char	_buf[4096];	/* Not thread-safe but sufficient for our
					 * our purpose. In a multi-threaded application
					 * each thread should have its own Error instance.
					 * and functions should not be static.
					 */

	static void	_report(const char* prefix, const char* file, const char* func, long line, 
			        const char* fmt, va_list vl);
};

/* Convenience wrappers around Error:X() functions
 */
#define WATCHMAN_ERROR(FMT, ...)	Error::error(__FILE__, __func__, __LINE__, FMT, ## __VA_ARGS__)
#define WATCHMAN_WARN(FMT, ...)		Error::warn(__FILE__, __func__, __LINE__, FMT, ## __VA_ARGS__)
#define WATCHMAN_LOG(FMT, ...)		Error::log(__FILE__, __func__, __LINE__, FMT, ## __VA_ARGS__)
#define WATCHMAN_DEBUG(FMT, ...)	Error::debug(__FILE__, __func__, __LINE__, FMT, ## __VA_ARGS__)

#endif

