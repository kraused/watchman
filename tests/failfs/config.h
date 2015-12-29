
#ifndef FAILFS_CONFIG_H_INCLUDED
#define FAILFS_CONFIG_H_INCLUDED 1

#include <linux/limits.h>

/* Maximal path length.
 */
#undef  FAILFS_PATH_MAXLEN
#define FAILFS_PATH_MAXLEN	PATH_MAX

#undef  FAILFS_MIRRORDIR
#define FAILFS_MIRRORDIR	"/dev/shm/failfs-mirrordir-%s"

#endif

