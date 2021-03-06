#ifndef QU_H
#define QU_H
#include "Filesys.h"
/*
extern const char filename[12];
extern const int inode_size,datablk_size, dir_size, indbmp_size, blkbmp_size, inodes_size;
*/
struct workdir_pathnode{
    int dir_inode;
    char dirname[252];
    workdir_pathnode *nextdir;
    workdir_pathnode *prevdir;
};

extern workdir_pathnode *wkpath;

#ifndef input_buffer_length
#define input_buffer_length 1036036
#endif
extern char inputbuffer[input_buffer_length];

void FormatDisk();
int InitDisk();
long DataBlkPos(int x);
long inodesPos(int x);
long DirsPos(int x); //返回第x子文件目录的相对位置
//WriteDir 第二个参数是子文件夹相对于父文件夹位置， 第三个参数是父文件夹的block_id，第四个参数是子文件夹的inode_id
void WriteDir(const char *dir_name, int relative_dir_entry_id, int dir_block_id, int son_dir_id);
//重命名$path的文件为AimedName path错误返回负值，AimedName已存在返回10
int ReName(char path[], char AimedName[]);
//命令行的前端函数
int WaitMessage();
bool IsExit(int);
int GetWorkDir();
//找到路径为$path的文件或文件夹,type_judge == 1时同时切换目录
int GetPathInode(char path[], int type_judge = 0);
//供GetPathInode调用的递归函数
int FindPath(char path[], int inode_id,int type_find = 0);
//列出工作目录下的内容
int ListDirs(char path[]);
//路径错误报错
void PathError(char path[]);
//文件已存在报错
void ExistedError(char path[]);
//文件已满报错
void FullError();
//文件名超长报错
void NameLongError();
//命令错误报错
void CmdError(char cmds[]);
//不是目录报错
void DirError(char path[]);
//文件不存在报错
void NoExistedErr(char path[]);
//输入为空报错
void EmptyErr();
//获得自身文件名
int GetSelfName(int inode_id, char selfname[]);
int UpdateInode(int x);
int UpdateIndBmp(int inode_bmp_id);
int UpdateBlkBmp(int block_bmp_id);
void debug();
int GetAboPath(char *path);
//切换目录
int ChangeDir(char *path);

//成功创建返回0， 路径错误返回-1， 已存在返回-2, 子文件夹满了返回-3
int MakeFolder(char path[]);
#endif
