
#ifndef FAILFS_COMPILER_H_INCLUDED
#define FAILFS_COMPILER_H_INCLUDED 1

/* compiler.h: Platform macros etc.
 *
 * Make sure this header file can be used with C and C++
 * code.
 */

#define likely(x)	__builtin_expect(!!(x), 1)
#define unlikely(x)	__builtin_expect(!!(x), 0)

#endif

