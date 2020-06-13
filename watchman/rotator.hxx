
#ifndef WATCHMAN_ROTATOR_HXX_INCLUDED
#define WATCHMAN_ROTATOR_HXX_INCLUDED 1

#include "watchman/config.hxx"

class File;


/* Interface for all implementations of log/file rotation.
 */
class Rotator
{

public:
				/* Returns true if the file can be rotated by
				 * this instance of a Rotator.
				 */
	virtual bool		file_is_supported(const File *f) const;

public:
				/* Returns true if the file can be rotated currently.
				 */
	virtual bool		file_can_be_rotated(File *f);

public:
				/* Returns true if the file should be rotated
				 * according to the rotation logic.
				 */
	virtual bool		file_requires_rotation(File *f) = 0;

public:
				/* Rotate the file.
				 */
	virtual int		file_rotate(File *f);

protected:
				/* Transform the file path of the file f to the new
				 * path used for storing the rotated file.
				 */
	virtual const char*	transform_file_path(File *f) = 0;

private:
	char			_path[WATCHMAN_PATH_MAX_LEN];

};

#endif

