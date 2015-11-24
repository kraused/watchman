
#include "file_pair.hxx"

File_Pair::File_Pair(int fd_o, int fd_e)
: _fd_o(fd_o), _fd_e(fd_e)
{
}

void File_Pair::set_stdout_fileno(int fd_o)
{
	_fd_o = fd_o;
}

void File_Pair::set_stderr_fileno(int fd_e)
{
	_fd_e = fd_e;
}

