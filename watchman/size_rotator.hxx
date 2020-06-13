
#ifndef WATCHMAN_SIZE_ROTATOR_HXX_INCLUDED
#define WATCHMAN_SIZE_ROTATOR_HXX_INCLUDED 1

#include "watchman/config.hxx"
#include "watchman/rotator.hxx"


/* A rotator implementation which rotates files exceeding a size threshold.
 */
class Size_Rotator : public Rotator
{

public:
				explicit Size_Rotator(long long threshold);

public:
				/* Returns true if the file should be rotated
				 * according to the rotation logic.
				 */
	virtual bool		file_requires_rotation(File *f);

protected:
				/* Transform the file path of the file f to the new
				 * path used for storing the rotated file.
				 */
	virtual const char*	transform_file_path(File *f);

private:
	long long		_threshold;
	char			_newpath[WATCHMAN_PATH_MAX_LEN];

};

#endif

