#include "fu.h"
const char filename[12] = "Filesys.vfs";
const int inode_size = 32, datablk_size = 4096, dir_size = 256;
const int indbmp_size = 4096, blkbmp_size = 4096, inodes_size = inode_size * 4096;
long DataBlkPos(int x); //返回第x个data_block在磁盘文件中的位置
long InodesPos(int x); //返回第x个inode在磁盘文件中的位置
long DirsPos(int x); //返回第x子文件目录的相对位置
int UpdateInode(int x);//更新第x个inode信息 
int UpdateBlkBmp(int x);//更新第x个超级块block信息 
int FindSonPath(char sonpath[],int inode_id); 
int FindPath(char path[], int inode_id);//返回path[]的inode_id路径或-1 
void WriteDir(const char *dir_name, int dir_id, int inode_id);//在inode_id文件夹中的第dir_id个位置建立文件夹联系dir_name 
//注意，WriteDir函数中写入dir_name没有把后面补全'\0' 
//建立的文件夹联系不代表存在这样一个文件夹，此函数十分危险
//此函数好像有问题，DirsPos(252)是不对的 
//以上来自qu.cpp 
_dir_block get_dirblock(block_id)//创建dirblock 
{
	
}
int find_free_indbmp(){
	int i = 0;
	for (; (i < indbmp_size) && (skbs.inode_bitmap[i] != 0); i++);
	if (i == 4096)
		return -1;
	return i;
}
int find_free_blkbmp(){
	int i = 0;
	for (; (i < blkbmp_size) && (skbs.block_bitmap[i] != 0); i++);
	if (i == 4096)
		return -1;
	return i;
}
int find_free_dir_entry(int inode_id){ 
	int block_id = inodes[inode_id].i_blocks[0];
	//判断是否为文件夹 
	_dir_block block1 = get_dirblock(block_id);//找到block
	int i = 0;
	for (; (i<16)&&(block->dirs.name[0] != '\0'); i++);
	if (i==16)
		return -1;
	return i;
}
int echo(char path[],int inode_id, char str[]) //未测试 
/*要不要考虑把当前inode_id设为全局变量，新建函数判断path为相对路径/绝对路径，结果返回根目录/当前目录 
另外，既然findpath既得到是否存在，又得到inode，不如去掉error函数放在各个命令实现里 
*/
{
	char path_up[252] = {0};
	int UpDirPos = 0;
	bool UpDir = 0;
	for (int i = 251; i >= 0; i--){
		if(path[i] == '/') {
            UpDir = 1;
            UpDirPos = i;
            break;
        }
	}
	if (UpDir){
		for (int i = UpDirPos - 1; i >= 0; i--){
			path_up[i] = path[i];
		}
		for (int i = UpDirPos; i < 252; i++){
			path_up[i] = '\0';
		}
	}
	int upstr_inode_id = FindPath(path_up[],inode_id);
	if (upstr_inode_id==-1){
		printf("%s No such directory\n",path_up[]);
		return -1;
	}//这里其实重复查找了，完成后可考虑修改FindPath函数 
	int str_inode_id = FindPath(path[],inode_id);
	if (str_inode_id==-1){
		int x = find_free_dir_entry(y);
		if (x==-1){
			printf("%s is full\n",path_up[]);
			return -1;
		}
		//创建path文件，将str_inode_id定义为创建的inode_id 
	}
	inodes[str_inode_id].i_blocks[0]
}
