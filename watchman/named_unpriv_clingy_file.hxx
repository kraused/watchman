
#ifndef WATCHMAN_NAMED_UNPRIV_CLINGY_FILE_HXX_INCLUDED
#define WATCHMAN_NAMED_UNPRIV_CLINGY_FILE_HXX_INCLUDED 1

#include "watchman/config.hxx"
#include "watchman/clingy_file.hxx"
#include "watchman/named_unpriv_file.hxx"

/* File with an unprivileged ower and an associated path on a filesystem. In contrast
 * to Named_Unpriv_File instances of Named_Unpriv_Clingy_File are associated with a particular 
 * filesystem and mountpoint and will not allow reopening if that filesystem is not
 * mounted where it is expected.
 */
typedef Clingy_File<Named_Unpriv_File> Named_Unpriv_Clingy_File;

#endif

