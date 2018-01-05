#ifndef FU_H
#define FU_H
#include "Filesys.h"
#include "filestruct.h"
/*
extern const char filename[12];
extern const int inode_size,datablk_size, dir_size, indbmp_size, blkbmp_size, inodes_size;

#ifndef input_buffer_length
#define input_buffer_length 1036036
#endif
*/
_dir_block get_dirblock(int inode_id);//创建dirblock，警告，使用前需要判断是否为目录，且需要保证目录名<252 
_file_block get_fileblock(int inode_id);//创建fileblock，警告，使用fileblock前需要判断是否为文件，且需要保障文件名大小<252
void write_fileblock_into_file(char str[], int block_id);//在block_id上书写str，警告，每次使用前需保证是文件

void BlockFullError();//block块已满
void InodeFullError();//inode块已满
void FileError(char path[]);//非文件错误

int find_free_indbmp();
int find_free_blkbmp();
int find_free_dir_entry(int inode_id); //满了返回-1
int find_position_dir_entry(int path_inode_id);

int cat(char path[]);//读取path路径的文件，-2不存在，-1目录，0成功
int DoCat(char path[], char data[]);//cat内核，对于路径path，修改data为path的数据，返回值-2不存在，-1目录，0成功

int rm(char path[]);//删除path路径的文件，-2不存在，-1目录，0成功
int rmdir(char path[]);//删除path路径的目录，-2不存在，-1文件，0成功

//extern char path_up[input_buffer_length];//echo用，上级路径
int echo(char path[], char str[]);//将str在写入path路径的文件，需求path以'\0'结尾，str随意
int DoEcho(char path[], char str[]);//echo内核，-k路径错误，+k空间错误，0成功
int cut_path_and_path_up(char path[], char str_name[]);//echo副函数，-1文件名过长，0成功

int delete_directory(int path_inode_id);//删除某inode_id的目录，-2不存在，-1文件，0成功
int delete_file(char path[]);//删除某inode_id的文件，-2不存在，-1文件夹，0成功
int delete_file(int path_inode_id);//删除某路径的文件，-2不存在，-1文件夹，0成功

int format();//磁盘格式化

void ModeError();//refresh格式错误
int refresh(int i);//刷新磁盘/程序 1表示刷新磁盘 0表示刷新磁盘
#endif


