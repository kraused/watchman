
#include <unistd.h>

#include "file.hxx"

File::File(int fd)
: _fd(fd), _can_reopen(false)
{
}

long long File::read(void *buf, long long nbyte)
{
	long long x;

	x = ::read(_fd, buf, nbyte);

	return x;
}

long long File::write(const void *buf, long long nbyte)
{
	long long x;

	x = ::write(_fd, buf, nbyte);

	return x;
}

int File::reopen()
{
	return -1;
}

