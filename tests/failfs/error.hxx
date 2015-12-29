
#ifndef FAILFS_ERROR_HXX_INCLUDED
#define FAILFS_ERROR_HXX_INCLUDED

#include <stdarg.h>

class Error
{

public:
	void	error(const char *file, const char *func, long line, const char *fmt, ...);
	void	warn (const char *file, const char *func, long line, const char *fmt, ...);
	void	log  (const char *file, const char *func, long line, const char *fmt, ...);
	void	debug(const char *file, const char *func, long line, const char *fmt, ...);

private:
	char	_buf[4096];

	void	_report(const char* prefix, const char* file, const char* func, long line,
		        const char* fmt, va_list vl);
};

Error *get_Error_instance_in_TLS();

/* Convenience wrappers around Error:X() functions
 */
#define FAILFS_ERROR(FMT, ...)	get_Error_instance_in_TLS()->error(__FILE__, __func__, __LINE__, FMT, ## __VA_ARGS__)
#define FAILFS_WARN(FMT, ...)	get_Error_instance_in_TLS()->warn(__FILE__, __func__, __LINE__, FMT, ## __VA_ARGS__)
#define FAILFS_LOG(FMT, ...)	get_Error_instance_in_TLS()->log(__FILE__, __func__, __LINE__, FMT, ## __VA_ARGS__)
#define FAILFS_DEBUG(FMT, ...)	get_Error_instance_in_TLS()->debug(__FILE__, __func__, __LINE__, FMT, ## __VA_ARGS__)

#endif

