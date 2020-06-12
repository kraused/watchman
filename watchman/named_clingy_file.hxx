
#ifndef WATCHMAN_NAMED_CLINGY_FILE_HXX_INCLUDED
#define WATCHMAN_NAMED_CLINGY_FILE_HXX_INCLUDED 1

#include "watchman/config.hxx"
#include "watchman/clingy_file.hxx"
#include "watchman/named_file.hxx"

/* File with an associated path on a filesystem. In contrast to Named_File instances
 * of Named_Clingy_File are associated with a particular filesystem and mountpoint and
 * will not allow reopening if that filesystem is not mounted where it is expected.
 */
typedef Clingy_File<Named_File> Named_Clingy_File;

#endif

