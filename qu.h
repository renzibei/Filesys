#ifndef QU_H
#define QU_H
#include "Filesys.h"

extern const char filename[12];
extern const int inode_size,datablk_size, dir_size, indbmp_size, blkbmp_size, inodes_size;

int InitDisk();

#endif
