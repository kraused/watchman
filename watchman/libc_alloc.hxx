
#ifndef WATCHMAN_LIBC_ALLOC_HXX_INCLUDED
#define WATCHMAN_LIBC_ALLOC_HXX_INCLUDED 1

#include "watchman/alloc.hxx"

class Libc_Allocator : public Allocator
{

public:
	void			*alloc(long long size);
	void			*free(void *ptr);

};

#endif

