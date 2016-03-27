
#ifndef WATCHMAN_COMPILER_HXX_INCLUDED
#define WATCHMAN_COMPILER_HXX_INCLUDED 1

#define likely(x)	__builtin_expect(!!(x), 1)
#define unlikely(x)	__builtin_expect(!!(x), 0)

#endif

