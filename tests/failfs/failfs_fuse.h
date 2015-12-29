
#ifndef FAILFS_FAILFS_FUSE_H_INCLUDED
#define FAILFS_FAILFS_FUSE_H_INCLUDED 1

#ifdef __cplusplus
extern "C"
{
#endif

/* Initialize the fuse library and mount the filesystem.
 */
int failfs_fuse_init(const char *mountpoint, void *failfs);

/* Finalize the fuse library. Unmount the filesystem.
 */
int failfs_fuse_fini();

int failfs_fuse_loop();

#ifdef __cplusplus
}
#endif

#endif

