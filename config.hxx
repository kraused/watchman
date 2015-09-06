
#ifndef WATCHMAN_CONFIG_HXX_INCLUDED
#define WATCHMAN_CONFIG_HXX_INCLUDED 1

/* Maximal number of children that can be watched at a time.
 */
#undef  WATCHMAN_MAX_CHILDREN
#define WATCHMAN_MAX_CHILDREN 	1

/* Maximal number of poll() failures in a row that will be
 * tolerated.
 */
#undef  WATCHMAN_MAX_POLL_FAILS
#define WATCHMAN_MAX_POLL_FAILS	2

/* Timeout argument for the poll() call
 */
#undef	WATCHMAN_POLL_TIMEOUT
#define	WATCHMAN_POLL_TIMEOUT	-1

/* How many seconds to wait for the children to terminate before
 * killing them. The shutdown of the application can take up to
 * two times this value.
 */
#undef	WATCHMAN_ALARM_SECS
#define	WATCHMAN_ALARM_SECS	1

/* Maximal string length in the argument list of a program (including
 * the path to to the program itself)
 */
#undef  WATCHMAN_PROGRAM_MAX_ARGV_STRLEN
#define WATCHMAN_PROGRAM_MAX_ARGV_STRLEN	128

/* Maximal number of arguments for a program (including the path
 * to the program)
 */
#undef	WATCHMAN_PROGRAM_MAX_ARGC
#define WATCHMAN_PROGRAM_MAX_ARGC	16

#endif

