#ifndef QU_H
#define QU_H
#include "Filesys.h"

extern const char filename[12];
extern const int inode_size,datablk_size, dir_size, indbmp_size, blkbmp_size, inodes_size;

struct workdir_pathnode{
    int dir_inode;
    char dirname[252];
    workdir_pathnode *nextdir;
    workdir_pathnode *prevdir;
};

int InitDisk();
int WaitMessage();
bool IsExit(int);
int GetWorkDir();
int GetPathInode(char path[], int type_judge = 0);
int FindPath(char path[], int inode_id,int type_find = 0);
void PathError(char path[]);
void PathError(char path[]);
#endif
