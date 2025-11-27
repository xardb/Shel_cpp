#ifndef VFS_H
#define VFS_H

#define FUSE_USE_VERSION 35

#include <fuse3/fuse.h>

extern struct fuse_operations users_operations;
void init_users_operations();

#endif
