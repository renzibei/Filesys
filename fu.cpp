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
		fread(block.dirs[i].name, 252, 1, vfs);
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
int find_position_dir_entry(int path_inode_id){//未加path判断，即需自行判断path存在且uppath为文件夹
	int uppath_inode_id = inodes[path_inode_id].fat_id;
	_dir_block block1 = get_dirblock(uppath_inode_id);//找到block
	int i = 0;
	for (; (i < 16) && (block1.dirs[i].inode_id == path_inode_id); i++);
	if (i==16){
		return -1;
	}
	return i;
}

int echo(char path[], char str[])//将str在写入path路径的文件 
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
		printf("%s No such file or directory\n", path_up);
		return -1;
	}
	int upstr_inode_id = GetPathInode(path_up);
	if (upstr_inode_id<0){
		printf("%s No such file or directory\n", path_up);
		return -1;
	}
	if (inodes[upstr_inode_id].i_mode == 0){
		printf("%s is not directory\n",path_up);
		return -1;
	}

	int str_inode_id = FindPath(path, upstr_inode_id);//若不存在，则创建基本信息
	if (str_inode_id<0){
		int str_position = find_free_dir_entry(upstr_inode_id, path_up);
		if (str_position < 0){
			return -1;
		} 
		str_inode_id = find_free_indbmp();
		if (str_inode_id<0){
			return -1;
		}
		int str_block_id = find_free_blkbmp();
		if (str_block_id<0){
			return -1;
		}//以上对应磁盘空间已满的情况

		char path_name[252] = {0};
		WriteDir(str_name, str_position, upstr_inode_id, str_inode_id);
		inodes[str_inode_id] = _inode(str_inode_id, 1, sizeof(str), upstr_inode_id, str_block_id);
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
 
int cat(char path[])//读取path路径的文件
{
	int str_inode_id = GetPathInode(path); 
	if (str_inode_id<0){
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
int delete_file(int path_inode_id)//删除某inode_id的文件，已加判断path是否为文件夹，需自行判断path存在且uppath为文件夹
{
	if (inodes[path_inode_id].i_mode == 0) {
		return -1;
	}//路径为文件夹

	sbks.inode_bitmap[path_inode_id] = 0;
	UpdateIndBmp(path_inode_id);//重置path对应的inode_bitmap

	int path_block_id = inodes[path_inode_id].i_blocks[0];
	sbks.inode_bitmap[path_block_id] = 0;
	UpdateBlkBmp(path_inode_id);//重置path对应的block_bitmap

	char str[252] = { 0 };
	for (int i = 0; i < 252; i++) {
		str[i] = '\0';
	}
	write_fileblock_into_file(str, path_block_id);//删除path的block

	int uppath_inode_id = inodes[path_block_id].fat_id;
	int dir_id = find_position_dir_entry(path_inode_id);
	WriteDir(str, dir_id, 0, uppath_inode_id);//删除path对应的dir_entry

	inodes[path_inode_id] = _inode(0, 0, 0, 0, 0);
	UpdateInode(path_inode_id);//删除path的inode
	return 0;
}

int delete_directory(int path_inode_id)//删除某inode_id的文件，path_inode_id<0代表不存在，返回-2；若是文件（而非文件夹）返回-1
{
	if (path_inode_id < 0) {
		return -2;
	}
	if (inodes[path_inode_id].i_mode == 1) {
		return -1;
	}

	_dir_block dirblock = get_dirblock(path_inode_id);
	//删除子文件夹和子文件
	for (int i = 3; i < 16; i++) {
		
		char* down_name = dirblock.dirs[i].name;
		bool NotEmpty = false;
		for (int j = 0; j < 252; j++) {
			if (down_name[j] != 0) {
				NotEmpty = true;
				break;
			}
		}
		if (NotEmpty) {
			int down_inode_id = dirblock.dirs[i].inode_id;
			int down_mode = inodes[down_inode_id].i_mode;
			if (down_mode == 1) {
				delete_file(down_inode_id);
			}
			else {
				delete_directory(down_inode_id);
			}
		}
	}

	char str[252] = { 0 };
	for (int i = 0; i < 252; i++) {
		str[i] = '\0';
	}//current here 12/12 11:38
	int uppath_inode_id = inodes[path_inode_id].fat_id;
	int path_position = find_position_dir_entry(path_inode_id);
	if (path_position < 0) {//理论上这种情况不能出现，debug完毕后要删去
		printf("Unexpected mistake happened");
		return -1;
	}
	if (path_position >= 0) {
		WriteDir(str, path_position, 0, uppath_inode_id);
	}
	WriteDir(str, 1, 0, path_inode_id);
	WriteDir(str, 0, 0, path_inode_id);//删除自身目录项的"." ".." 以及上一级文件夹目录项的"./name"

	sbks.inode_bitmap[path_inode_id] = 0;
	UpdateIndBmp(path_inode_id);
	int path_block_id = inodes[path_inode_id].i_blocks[0];
	sbks.inode_bitmap[path_block_id] = 0;
	UpdateIndBmp(path_block_id);//解除superblock占用状态

	inodes[path_inode_id] = _inode(0, 0, 0, 0, 0);
	UpdateInode(path_inode_id);//删除自身inode信息

	return 0;
}



int rm(char path[])//删除path路径的文件 
{
	int path_inode_id = GetPathInode(path);
	if (path_inode_id < 0) {
		printf("%s No such file or directory\n", path);
		return -1;
	}
	int i = delete_file(path_inode_id);//这里自然path存在，uppath存在且是文件夹
	if (i < 0) {
		printf("%s is not a file\n", path);
	}
	return i;
}

int rmdir(char path[])//删除path路径的文件夹
{
	int path_inode_id = GetPathInode(path);
	int i = delete_directory(path_inode_id);
	if (i < 0) {
		if (i == -2)
			printf("%s No such file or directory\n", path);
		if (i == -1)
			printf("%s is not a directory\n", path);
		return -1;
	}
	return i;
}
