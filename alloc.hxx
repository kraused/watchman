
#ifndef WATCHMAN_ALLOC_HXX_INCLUDED
#define WATCHMAN_ALLOC_HXX_INCLUDED 1

#include <new>

#include "compiler.hxx"

class Allocator
{

public:
	virtual void		*alloc(long long size) = 0;
	virtual void		*free(void *ptr) = 0;

public:
	template<class T>
	T			*create();

	template<class T, typename TypeArg1>
	T			*create(const TypeArg1 &arg1);

	template<class T, typename TypeArg1, typename TypeArg2>
	T			*create(const TypeArg1 &arg1, const TypeArg2 &arg2);

	template<class T>
	T			*destroy(T *ptr);

};

template<class T>
T *Allocator::create()
{
	return new(alloc(sizeof(T))) T;
}

template<class T, typename TypeArg1>
T *Allocator::create(const TypeArg1 &arg1)
{
	return new(alloc(sizeof(T))) T(arg1);
}

template<class T, typename TypeArg1, typename TypeArg2>
T *Allocator::create(const TypeArg1 &arg1, const TypeArg2 &arg2)
{
	return new(alloc(sizeof(T))) T(arg1, arg2);
}

template<class T>
T *Allocator::destroy(T *ptr)
{
	ptr->~T();
	return (T *)free(ptr);
}

#endif

