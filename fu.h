#ifndef FU_H
#define FU_H
#include "Filesys.h"
#include "filestruct.h"
extern const char filename[12];
extern const int inode_size,datablk_size, dir_size, indbmp_size, blkbmp_size, inodes_size;

int find_free_indbmp();
int find_free_blkbmp();
int cat(char path[]);
int rm(char path[]);
int rmdir(char path[]);
#endif


