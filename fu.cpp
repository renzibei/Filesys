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

//前端需求：$str不超过4095个字符（即算上'\0'不能越位）

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
void write_fileblock_into_file(char str[4096],int block_id)//在block_id上书写str，警告，每次使用前需保证是文件 
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
int find_position_dir_entry(int inode_id, char dir_name[252],char uppath[]){
	if (inodes[inode_id].i_mode == 1){
		printf("%s is not a directory\n",uppath);
		return -1;
	}
	int block_id = inodes[inode_id].i_blocks[0]; 
	_dir_block block1 = get_dirblock(inode_id);//找到block
	int i = 0;
	for (; i<16; i++){
		bool Same = true;
		for(int j = 0; j<252;j++){
			if (block1.dirs[i].name[j] != dir_name[j]){
				Same = false;
				break;
			}
		}
		if (Same){
			break;
		}
	}
	if (i==16){
		printf("%s/%s No such file or directory\n", uppath, dir_name);
		return -1;
	}
	return i;
}

//将str在写入path路径的文件 
int echo(char path[], char str[])
{
	char path_up[252] = {0};//上一级目录
	char str_name[252] = {0};//文件名
	int UpDirPos = 0;
	bool UpDir = 0;
	for (int i = 251; i >= 0; i--){
		if(path[i] == '/') {
            UpDir = 1;
            UpDirPos = i;
            break;
        }
	}
	if (UpDir){//获得文件名、上级文件夹路径
		for (int i = UpDirPos - 1; i >= 0; i--){
			path_up[i] = path[i];
			str_name[251-i] = '\0';
		}
		for (int i = UpDirPos; i < 252; i++){
			path_up[i] = '\0';
			str_name[i-UpDirPos] = path[i];
		}
	}

	else {//意外情况
		printf("%s No such file or directory\n", path);
		return -1;
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

	int str_inode_id = FindPath(path, upstr_inode_id);//若不存在，则创建基本信息
	if (str_inode_id==-1){
		int str_position = find_free_dir_entry(upstr_inode_id, path_up);
		if (str_position < 0){
			return -1;
		} 
		str_inode_id = find_free_indbmp();
		if (str_inode_id==-1){
			return -1;
		}
		int str_block_id = find_free_blkbmp();
		if (str_block_id==-1){
			return -1;
		}//以上对应磁盘空间已满的情况

		char path_name[252] = {0};
		WriteDir(str_name, str_position, upstr_inode_id, str_inode_id);
		inodes[str_inode_id].i_id = str_inode_id; 
		inodes[str_inode_id].i_mode = 1;
		inodes[str_inode_id].i_blocks[0] = str_block_id;
		inodes[str_inode_id].i_file_size = sizeof(str);
		//不必清除inode/block，因为它们本来就是空的 
		sbks.inode_bitmap[str_inode_id] = 1;
		sbks.block_bitmap[str_block_id] = 1;
		UpdateIndBmp(str_inode_id);
		UpdateBlkBmp(str_block_id);
		UpdateInode(str_inode_id);//创建基本信息
	}

	char full_str[4096];
	int i = 0;
	for (;str[i] != '\0';i++){
		full_str[i] = str[i];
	}
	for (;i<4096;i++){
		full_str[i] = '\0';
	}//扩展str到标准长度

	write_fileblock_into_file(full_str,inodes[str_inode_id].i_blocks[0]);//写入str
	return 0;
}

//读取path路径的文件 
int cat(char path[])
{
	int str_inode_id = GetPathInode(path); 
	if (str_inode_id==-1){
		printf("%s No such file or directory\n",path);
		return -1;
	}//路径不存在
	if (inodes[str_inode_id].i_mode == 0) {
		printf("%s is not a file\n", path);
		return -1;
	}//路径为文件夹
	printf("%s\n",get_fileblock(str_inode_id).data);
	return 0;
}

//删除path路径的文件 
int rm(char path[])
{
	int path_inode_id = GetPathInode(path);
	if (path_inode_id == -1) {
		printf("%s No such file or directory\n", path);
		return -1;
	}//路径不存在
	if (inodes[path_inode_id].i_mode == 0) {
		printf("%s is not a file\n", path);
		return -1;
	}//路径为文件夹

	sbks.inode_bitmap[path_inode_id] = 0;
	UpdateIndBmp(path_inode_id);//重置path对应的inode_bitmap

	int path_block_id = inodes[path_inode_id].i_blocks[0];
	sbks.inode_bitmap[path_block_id] = 0;
	UpdateBlkBmp(path_inode_id);//重置path对应的block_bitmap

	inodes[path_inode_id].i_id = 0;
	inodes[path_inode_id].i_mode = 0;
	inodes[path_inode_id].i_blocks[0] = 0;
	inodes[path_inode_id].i_file_size = 0;
	//inodes[path_inode_id].i_place_holder = { 0 };
	UpdateInode(path_inode_id);//删除path的inode

	char str[252] = { 0 };
	for (int i = 0; i < 252; i++) {
		str[i] = '\0';
	}
	int dir_id = find_position_dir_entry(path_inode_id, path_name, path_up);
	write_fileblock_into_file(str, path_block_id);//删除path的block

	WriteDir(str, dir_id, 0, uppath_inode_id);//删除path对应的dir_entry
	return 0;
}
