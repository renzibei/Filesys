#include "fu.h"
#include "qu.cpp"
/*const char filename[12] = "Filesys.vfs";
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
int UpdateIndBmp(int x);
int UpdateBlkBmp(int x);
int UpdateInode(int x);
int GetPathInode(char path[], int type_judge = 0); */
//注意，WriteDir函数中写入dir_name没有把后面补全'\0' 
//以上来自qu.cpp 

//_super_block sbks;
//_inode inodes[4096];

_dir_block get_dirblock(int inode_id)//创建dirblock，警告，每次使用dirblock之前都需要判断是否为文件夹 
{
    FILE *vfs = fopen(filename, "rb+");
    _dir_block block;
    int block_id = inodes[inode_id].i_blocks[0];
    int Position = DataBlkPos(block_id);
    for (int i = 0; i < 16; i++){
    	fseek(vfs, Position + i * dir_size, SEEK_SET);
    	fread(block.dirs[i].name,sizeof(block.dirs[i].name), 1, vfs);
    	fseek(vfs, 252, SEEK_CUR);
    	fread(&block.dirs[i].inode_id,sizeof(block.dirs[i].inode_id), 1, vfs);
    }
    fclose(vfs);
    return block;
}
_file_block get_fileblock(int inode_id)//创建fileblock，警告，每次使用fileblock之前都需要判断是否为文件 
{
	_file_block fileblock;
    FILE *vfs = fopen(filename, "rb+");
    int block_id = inodes[inode_id].i_blocks[0];
    int Position = DataBlkPos(block_id);
    fseek(vfs, Position, SEEK_SET);
    fread(fileblock.data ,sizeof(fileblock.data), 1, vfs);
    fclose(vfs);
    return fileblock;
}
void write_fileblock_into_file(char str[],int block_id)//在block_id上书写str，警告，每次使用前需保证是文件 
{
    FILE *vfs = fopen(filename, "rb+");
    int Position = DataBlkPos(block_id);
    fseek(vfs, Position, SEEK_SET);
    fwrite(str ,sizeof(str), 1, vfs);
    fwrite('\0' ,sizeof(char), datablk_size-sizeof(str), vfs);
    fclose(vfs);
    return;
}
int find_free_indbmp(){
	int i = 0;
	for (; (i < indbmp_size) && (sbks.inode_bitmap[i] != 0); i++);
	if (i == 4096){
		printf("All inodes are used\n");
		return -1;
	}
	return i;
}
int find_free_blkbmp(){
	int i = 0;
	for (; (i < blkbmp_size) && (sbks.block_bitmap[i] != 0); i++);
	if (i == 4096){
		printf("All blocks are used\n");
		return -1;
	}
	return i;
}
int find_free_dir_entry(int inode_id, char path[]){
	if (inodes[inode_id].i_mode == 1){
		printf("%s is not a directory\n",path);
	}
	int block_id = inodes[inode_id].i_blocks[0]; 
	_dir_block block1 = get_dirblock(inode_id);//找到block
	int i = 0;
	for (; (i<16)&&(block1.dirs[i].name[0] != '\0'); i++);
	if (i==16){
		printf("%s is full\n",path);
		return -1;
	}
	return i;
}

//将str在写入path路径的文件 
int echo(char path[],int inode_id, char str[])
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
	int upstr_inode_id = GetPathInode(path_up);
	if (upstr_inode_id==-1){
		printf("%s No such directory\n",path_up);
		return -1;
	}
	if (inodes[upstr_inode_id].i_mode == 0){
		printf("%s is not directory\n",path_up);
		return -1;
	} 
	int str_inode_id = FindPath(path, upstr_inode_id);
	if (str_inode_id==-1){
		int x = find_free_dir_entry(upstr_inode_id, path_up);
		if (x < 0){
			return -1;
		} 
		str_inode_id = find_free_indbmp();
		if (str_inode_id==-1){
			return -1;
		}
		int str_block_id = find_free_blkbmp();
		if (str_block_id==-1){
			return -1;
		}
		char path_name[252] = {0};
		inodes[str_inode_id].i_id = str_inode_id; 
		inodes[str_inode_id].i_mode = 1;
		inodes[str_inode_id].i_blocks[0] = str_block_id;
		inodes[str_inode_id].i_file_size = sizeof(str);
		//不必清除inode/block，因为它们本来就是空的 
		sbks.inode_bitmap[str_inode_id] = 1;
		sbks.block_bitmap[str_block_id] = 1;
		UpdateIndBmp(str_inode_id);
		UpdateBlkBmp(str_block_id);
		UpdateInode(str_inode_id);
	}
	write_fileblock_into_file(str,inodes[inode_id].i_blocks[0]);
	return 0;
}

//读取path路径的文件 
int cat(char path[],int inode_id)
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
	int upstr_inode_id = GetPathInode(path_up);
	if (upstr_inode_id==-1){
		printf("%s No such directory\n",path_up);
		return -1;
	}
	if (inodes[upstr_inode_id].i_mode == 0){
		printf("%s is not directory\n",path_up);
		return -1;
	}
	int str_inode_id = FindPath(path, upstr_inode_id);//这里也是重复查找 
	if (str_inode_id==-1){
		printf("%s No such file\n",path);
		return -1;
	}
	printf("%s\n",get_fileblock(str_inode_id).data);
	return 0;
}

//删除path路径的文件 
int rm(char path[],int inode_id)
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
	int upstr_inode_id = GetPathInode(path_up);
	if (upstr_inode_id==-1){
		printf("%s No such directory\n",path_up);
		return -1;
	}
	if (inodes[upstr_inode_id].i_mode == 0){
		printf("%s is not directory\n",path_up);
		return -1;
	}
	int str_inode_id = FindPath(path, upstr_inode_id);
	if (str_inode_id==-1){
		printf("%s No such file\n",path);
		return -1;
	}
	
	return 0;
}
