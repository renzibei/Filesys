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

extern workdir_pathnode *wkpath;

#ifndef input_buffer_length
#define input_buffer_length 4096
#endif
extern char inputbuffer[input_buffer_length];

int InitDisk();
long DataBlkPos(int x);
long inodesPos(int x);
long DirsPos(int x); //返回第x子文件目录的相对位置
void WriteDir(const char *dir_name, int relative_dir_entry_id, int dir_block_id, int son_dir_id);
int WaitMessage();
bool IsExit(int);
int GetWorkDir();
int GetPathInode(char path[], int type_judge = 0);
int FindPath(char path[], int inode_id,int type_find = 0);
void PathError(char path[]);
void PathError(char path[]);
int UpdateInode(int x);
int UpdateIndBmp(int inode_bmp_id);
int UpdateBlkBmp(int block_bmp_id);
void debug();
int GetAboPath(char *path);
int ChangeDir(char *path);

//成功创建返回0， 路径错误返回-1， 已存在返回-2, 子文件夹满了返回-3
int MakeFolder(char path[]);
#endif
