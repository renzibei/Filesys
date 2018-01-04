#ifndef FU_H
#define FU_H
#include "Filesys.h"
#include "filestruct.h"
extern const char filename[12];
extern const int inode_size,datablk_size, dir_size, indbmp_size, blkbmp_size, inodes_size;

void BlockFullError();
void InodeFullError();
int find_free_indbmp();
int find_free_blkbmp();
int cat(char path[]);
int rm(char path[]);
int rmdir(char path[]);
int echo(char path[], char str[]);
int delete_directory(int path_inode_id);//删除某inode_id的文件，path_inode_id<0代表不存在，返回-2；若是文件（而非文件夹）返回-1
int delete_file(int path_inode_id);//删除某inode_id的文件，已加判断path是否为文件夹，需自行判断path存在且uppath为文件夹
#endif


