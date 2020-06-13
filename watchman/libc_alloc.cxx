
#include <stdlib.h>

#include "libc_alloc.hxx"

void *Libc_Allocator::alloc(long long size)
{
	return ::malloc(size);
}

void *Libc_Allocator::free(void *ptr)
{
	::free(ptr);
	return nullptr;
}

