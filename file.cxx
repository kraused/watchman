
#include <unistd.h>

#include "file.hxx"

File::File(int fd)
: _fd(fd), _can_reopen(false)
{
}

long long File::read(void *buf, long long nbyte)
{
	return ::read(_fd, buf, nbyte);
}

long long File::write(const void *buf, long long nbyte)
{
	return ::write(_fd, buf, nbyte);
}

int File::reopen()
{
	return -1;
}

