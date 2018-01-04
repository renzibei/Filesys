#define _CRT_SECURE_NO_WARNINGS
#include "fu.h"

//注意，WriteDir函数中写入dir_name没有把后面补全'\0' 
//以上来自qu.cpp 

//前端需求：$str不超过4095个字符（即算上'\0'不能越位）

_dir_block get_dirblock(int inode_id)//创建dirblock，警告，使用前需要判断是否为目录，且需要保证目录名<252 
{
	FILE *vfs = fopen(filename, "rb+");
	_dir_block block;
	int block_id = inodes[inode_id].i_blocks[0];
	long Position = DataBlkPos(block_id);
	for (int i = 0; i < 16; i++){
		fseek(vfs, Position + i * dir_size, SEEK_SET);
		fread(block.dirs[i].name, 252, 1, vfs);
		fread(&block.dirs[i].inode_id,sizeof(block.dirs[i].inode_id), 1, vfs);
	}
	fclose(vfs);
	return block;
}
_file_block get_fileblock(int inode_id)//创建fileblock，警告，使用fileblock前需要判断是否为文件，且需要保障文件名大小<252 
{
	_file_block fileblock;
	FILE *vfs = fopen(filename, "rb+");
	int block_id = inodes[inode_id].i_blocks[0];
	long Position = DataBlkPos(block_id);
	fseek(vfs, Position, SEEK_SET);
	fread(fileblock.data ,sizeof(fileblock.data), 1, vfs);
	fclose(vfs);
	return fileblock;
}

void write_fileblock_into_file(char str[],int block_id)//在block_id上书写str，警告，每次使用前需保证是文件
{
	FILE *vfs = fopen(filename, "rb+");
	long Position = DataBlkPos(block_id);
	fseek(vfs, Position, SEEK_SET);
	int lenstr = (int)strlen(str);
	if (lenstr > 4096) {
		lenstr = 4096;
	}
    fwrite(str ,sizeof(char), lenstr, vfs);
	char _zero[1] = { '\0' };
	for(int i=0;i<datablk_size - lenstr;i++)
		fwrite(_zero ,sizeof(char), 1, vfs);
	fclose(vfs);
	return;
}

void FileError(char path[]) {
	cout << path << " " << "is not a file." << endl;
}

void InodeFullError() {
	printf("All inodes are used\n");
}

int find_free_indbmp(){
	int i = 0;
	for (; (i < indbmp_size) && (sbks.inode_bitmap[i] != 0); i++);
	if (i == 4096){
		return -1;
	}
	return i;
}


void BlockFullError() {
	printf("All blocks are used\n");
}

int find_free_blkbmp(){
	int i = 0;
	for (; (i < blkbmp_size) && (sbks.block_bitmap[i] != 0); i++);
	if (i == 4096){
		return -1;
	}
	return i;
}
int find_free_dir_entry(int inode_id, char path[]) {
	if (inodes[inode_id].i_mode == 1){
		return -2;
	}
	int block_id = inodes[inode_id].i_blocks[0]; 
	_dir_block block1 = get_dirblock(inode_id);//找到block
	int i = 0;
	for (; (i<16)&&(block1.dirs[i].name[0] != '\0'); i++);
	if (i==16){
		return -1;
	}
	return i;
}

int find_position_dir_entry(int path_inode_id){//未加path判断，即需自行判断path存在
	int uppath_inode_id = inodes[path_inode_id].fat_id;
	_dir_block block1 = get_dirblock(uppath_inode_id);//找到block
	int i = 0;
	for (; (i < 16) && (block1.dirs[i].inode_id != path_inode_id); i++);
	if (i==16){
		return -1;
	}
	return i;
}

int echo(char path[], char str[])//将str在写入path路径的文件，需求path以'\0'结尾，str随意
{
	return 0;
}

int DoEcho(char path[], char str[])//echo内核
{
	int tmplen = (int)strlen(path);//路径长度
	char str_name[252] = {0};//文件名
	int UpDirPos = -1;//上级目录路径终止位置，即'/'位置
	bool flag = 0;//文件为标准格式"a/b/c"或非标准格式"a"(相当于"./a")前者为1，后者0
	for (int i = tmplen; i >= 0; i--){
		if(path[i] == '/') {
			flag = 1;
			UpDirPos = i;
			break;
		}
	}

	//获得文件名、上级文件夹路径
	int lenstr = tmplen;//str长度
	if (flag) {
		lenstr = tmplen - UpDirPos - 1;
	}
	if (lenstr > 251) {//文件名过长
		NameLongError();
		return -2;
	}
	char * path_up = new char[tmplen + 2];//上一级目录
	if (flag) {
		strncpy(path_up, path, UpDirPos);
		path_up[UpDirPos] = '\0';
		//cout << "path_up: " << path_up << endl;
	}
	else {
		path_up[0] = '.';
		path_up[1] = '\0';
	}
	strncpy(str_name, path + UpDirPos + 1, lenstr);
	str_name[lenstr] = '\0';
	//cout << "str_name: " << str_name << endl;


	int upstr_inode_id = GetPathInode(path_up);
	if (upstr_inode_id<0){
		PathError(path_up);
		delete[] path_up;
		return -1;
	}
	if (inodes[upstr_inode_id].i_mode == 1){
		DirError(path_up);
		delete[] path_up;
		return -1;
	}

	int str_inode_id = FindPath(path, upstr_inode_id);//inode_id，不存在为-1
	if (str_inode_id<0){//不存在则创建基本信息
		//寻找可用位置
		int str_position = find_free_dir_entry(upstr_inode_id, path_up);
		if (str_position < 0){
			FullError();
			delete[] path_up;
			return -1;
		} 
		str_inode_id = find_free_indbmp();
		if (str_inode_id<0){
			InodeFullError();
			delete[] path_up;
			return -1;
		}
		int str_block_id = find_free_blkbmp();
		if (str_block_id<0){
			BlockFullError();
			delete[] path_up;
			return -1;
		}//以上对应磁盘空间已满的情况

		//初始化
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
	for (;(str[i] != '\0') && i < 4096;i++){
		full_str[i] = str[i];
	}
	//cout << i << " i\n";
	for (;i<4096;i++){
		full_str[i] = '\0';
	}//扩展str到标准长度
	write_fileblock_into_file(full_str,inodes[str_inode_id].i_blocks[0]);//写入str
	delete[] path_up;
	return 0;
}
 
int cat(char path[]) {//读取path路径的文件，-2不存在，-1目录，0成功
	int i = DoCat(path);
	if (i == -2) {
		PathError(path);
	}
	else if (i == -1) {
		FileError(path);
	}
	return i;
}

int DoCat(char path[])//cat内核
{
	int str_inode_id = GetPathInode(path); 
	if (str_inode_id<0){
		return -2;
	}//路径不存在
	if (inodes[str_inode_id].i_mode == 0) {
		return -1;
	}//路径为目录
	_file_block block = get_fileblock(str_inode_id);
	char data[4097];
	int i = 0;
	for (; i < 4096 && block.data[i] != '\0'; i++) {
		data[i] = block.data[i];
	}
	data[i] = '\0';
	printf("%s\n",data);
	return 0;
}

int delete_file(char path[]) {//删除某inode_id的文件，-2不存在，-1文件夹，0成功
	int path_inode_id = GetPathInode(path);
	int i = delete_file(path_inode_id);
	return i;
}

int delete_file(int path_inode_id)//删除某inode_id的文件，-2不存在，-1文件夹，0成功
{
	if (path_inode_id<0) {
		return -2;
	}//路径不存在
	if (inodes[path_inode_id].i_mode == 0) {
		return -1;
	}//路径为文件夹

	sbks.inode_bitmap[path_inode_id] = 0;
	UpdateIndBmp(path_inode_id);//重置path对应的inode_bitmap

	int path_block_id = inodes[path_inode_id].i_blocks[0];
	sbks.block_bitmap[path_block_id] = 0;
	UpdateBlkBmp(path_inode_id);//重置path对应的block_bitmap

	char str[4096];
	for (int i = 0; i < 4096; i++) {
		str[i] = '\0';
	}
	write_fileblock_into_file(str, path_block_id);//删除path的block

	char file_name[252];
	for (int i = 0; i < 252; i++) {
		file_name[i] = '\0';
	}
	int uppath_inode_id = inodes[path_block_id].fat_id;
	int dir_id = find_position_dir_entry(path_inode_id);
	WriteDir(file_name, dir_id, uppath_inode_id, 0);//删除path对应的dir_entry

	inodes[path_inode_id] = _inode(0, 0, 0, 0, 0);
	UpdateInode(path_inode_id);//删除path的inode

	return 0;
}

int delete_directory(int path_inode_id)//删除某inode_id的目录，-2不存在，-1文件，0成功
{
	if (path_inode_id < 0) {
		return -2;
	}
	if (inodes[path_inode_id].i_mode == 1) {
		return -1;
	}

	_dir_block dirblock = get_dirblock(path_inode_id);
	//删除子文件夹和子文件
	for (int i = 2; i < 16; i++) {
		
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
		str[i] = 0;
	}
	int uppath_inode_id = inodes[path_inode_id].fat_id;
	int uppath_block_id = inodes[uppath_inode_id].i_blocks[0];
	int path_block_id = inodes[path_inode_id].i_blocks[0];
	int path_position = find_position_dir_entry(path_inode_id);
	if (path_position < 0) {
		printf("Unexpected mistake happened\n");
		return -1;
	}
	if (path_position >= 0) {
        /*for (int i = 0; i < 252; i++) {
			cout << (int)str[i];
		}
        cout << endl;*/
		WriteDir(str, path_position, uppath_inode_id, 0);
	}
    //cout << "path_inode_id " << path_inode_id << endl;
	WriteDir(str, 1, path_block_id, 0);
	WriteDir(str, 0, path_block_id, 0);//删除自身目录项的"." ".." 以及上一级文件夹目录项的"./name"

	sbks.inode_bitmap[path_inode_id] = 0;
	UpdateIndBmp(path_inode_id);
    //cout << "path_block_id " << path_block_id << endl;
	sbks.block_bitmap[path_block_id] = 0;
	UpdateBlkBmp(path_block_id);//解除superblock占用状态

	inodes[path_inode_id] = _inode(0, 0, 0, 0, 0);
	UpdateInode(path_inode_id);//删除自身inode信息

	return 0;
}

int rm(char path[])//删除path路径的文件，-2不存在，-1目录，0成功 
{
	int i = delete_file(path);
	if (i == -1) {

	}
	return i;
}

int rmdir(char path[])//删除path路径的目录，-2不存在，-1文件，0成功
{
	int path_inode_id = GetPathInode(path);
	int i = delete_directory(path_inode_id);
	if (i == -2) {
		PathError(path);
	}
	if (i == -1) {
		DirError(path);
	}
	return i;
}
