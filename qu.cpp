#define _CRT_SECURE_NO_WARNINGS
#include "qu.h"
#define EXITFLAG 0x7fffffff
const char filename[12] = "Filesys.vfs";
const int inode_size = 32, datablk_size = 4096, dir_size = 256;
const int indbmp_size = 4096, blkbmp_size = 4096, inodes_size = inode_size * 4096;
char inputbuffer[input_buffer_length];
char inputcontent[input_buffer_length] = { 0 };

workdir_pathnode *pathhead = NULL;
workdir_pathnode *wkpath = NULL;
workdir_pathnode *pathtail = NULL;
workdir_pathnode *temphead = NULL;
workdir_pathnode *tempwd   = NULL;
workdir_pathnode *temptail = NULL;
long DataBlkPos(int x) //返回第x个data_block在磁盘文件中的位置
{
    return indbmp_size + blkbmp_size + inodes_size + datablk_size * x;
}

long InodesPos(int x) //返回第x个inode在磁盘文件中的位置
{
    return indbmp_size + blkbmp_size + inode_size * x;
}

long DirsPos(int x) //返回第x子文件目录的相对位置
{
    return dir_size * x;
}

int UpdateIndBmp(int x)
{
    FILE *vfs = fopen(filename, "rb+");
    if(vfs == NULL)
        return -1;
    fseek(vfs, x, SEEK_SET);
    fwrite(&sbks.inode_bitmap[x], sizeof(bool), 1, vfs);
    fclose(vfs);
    return 0;
}

int UpdateBlkBmp(int x)
{
    FILE *vfs = fopen(filename, "rb+");
    if(vfs == NULL)
        return -1;
    fseek(vfs, x + indbmp_size, SEEK_SET);
    fwrite(&sbks.block_bitmap[x], sizeof(bool), 1, vfs);
    fclose(vfs);
    return 0;
}

void WriteDir(const char *dir_name, int relative_dir_entry_id, int dir_block_id, int son_dir_id)
{
	/*//cout << relative_dir_entry_id << " " << dir_block_id << " " << son_dir_id << endl;
	for (int i = 0; i < 252; i++) {
		cout << (int)dir_name[i];
	}*/
	//cout << endl;
    FILE *vfs = fopen(filename,"rb+");
    fseek(vfs,DataBlkPos(dir_block_id),SEEK_SET);
    fseek(vfs,DirsPos(relative_dir_entry_id),SEEK_CUR);
    int name_len = (int) strlen(dir_name);
    if(name_len == 0)
        name_len = 252;
    //cout << name_len << endl;
    fwrite(dir_name,sizeof(char), name_len, vfs);
    fseek(vfs,DataBlkPos(dir_block_id),SEEK_SET);
    fseek(vfs,DirsPos(relative_dir_entry_id),SEEK_CUR);
    fseek(vfs,252,SEEK_CUR);
    fwrite(&son_dir_id, sizeof(int), 1, vfs);
    fclose(vfs);
}

int UpdateInode(int x)
{
    FILE *vfs = fopen(filename,"rb+");
    fseek(vfs, InodesPos(x), SEEK_SET);
    fwrite(&inodes[x], sizeof(_inode), 1, vfs);
    /*
    fwrite(&inodes[x].i_id,sizeof(int), 3, vfs);
    fwrite(&inodes[x].i_blocks,sizeof(int),1,vfs);
    fwrite(inodes[x].i_place_holder,sizeof(char),PLACEHOLDER,vfs); */
    fclose(vfs);
    return 0;
}

int FindSonPath(char sonpath[],int inode_id, int &relasondir)
{
    FILE *vfs = fopen(filename, "rb");
    int dst_inode_id = -1, temp_inode_id = -1;
    bool existedfreeentry = 0;
    char pathname[252] = {0};
    for(int i = 0; i < 16; ++i) {
        fseek(vfs,DataBlkPos(inodes[inode_id].i_blocks[0]),SEEK_SET);
        fseek(vfs,DirsPos(i),SEEK_CUR);
        fread(pathname, sizeof(char), 252, vfs);
        if(strcmp(sonpath, pathname) == 0) {
            fread(&dst_inode_id, sizeof(int), 1, vfs);
            relasondir = i;
            return dst_inode_id;
		}
        fread(&temp_inode_id, sizeof(int), 1, vfs);
        if(temp_inode_id == 0 && !existedfreeentry && pathname[0] == 0) {
            existedfreeentry = 1;
            relasondir = i;
        }
    }
    fclose(vfs);
    if(!existedfreeentry)
        relasondir = -1;
    return -1;
}

void ExistedError(char path[])
{
    cout << path << " already existed." << endl;
}

void FullError()
{
    cout << "The Directory is full!" << endl;
}

void NameLongError()
{
    cout << "The Name should be no longer than 251 chars!" << endl;
}

char fat_path[input_buffer_length] = {0};
int MakeFolder(char path[])
{
    int path_len = (int) strlen(path), divpos = -1, fat_inode = 0;
    dir_name[253] = {0};
    for(int i = path_len-1; i > -1; --i)
        if(path[i] == '/') {
            divpos = i;
            break;
        }
    if(divpos == -1)
        fat_inode = GetWorkDir();
    else {
        strncpy(fat_path, path, divpos);
        fat_inode = GetPathInode(fat_path);
        if(fat_inode == -1) {
            //PathError(fat_path);
            return -1;
        }
    }
	strncpy(dir_name, path + divpos + 1, path_len - divpos -1);
    if(dir_name[252] != '\0')
        return -4;
        
    int rela_id = -1;
    if(FindSonPath(dir_name, fat_inode, rela_id) != -1) {
        //ExistedError(path);
        return -2;
    }
    if(rela_id == -1) {
       // cout << "The Directory is full!" << endl;
        return -3;
    }
    //记得加上满磁盘判断
    int new_dir_inode = find_free_indbmp();
    sbks.inode_bitmap[new_dir_inode] = 1;
    UpdateIndBmp(new_dir_inode);
    int new_blk_id = find_free_blkbmp();
    inodes[new_dir_inode] = _inode(new_dir_inode, 0, 0, fat_inode, new_blk_id);
    UpdateInode(new_dir_inode);
    sbks.block_bitmap[new_blk_id] = 1;
    UpdateBlkBmp(new_blk_id);
    WriteDir(dir_name, rela_id, inodes[fat_inode].i_blocks[0], new_dir_inode);
    WriteDir(".", 0, new_blk_id, new_dir_inode);
    WriteDir("..", 1, new_blk_id, fat_inode);
    return 0;
        
}

int MakeDir(char path[])
{
    int judgestatus = MakeFolder(path);
    if(judgestatus == -1)
        PathError(path);
    else if(judgestatus == -2)
        ExistedError(path);
    else if(judgestatus == -3)
        FullError();
    else if(judgestatus == -4)
        NameLongError();
    return judgestatus;
        
}

void MakeHome()
{
    
    sbks.inode_bitmap[0] = 1;
    sbks.block_bitmap[0] = 1;
    UpdateIndBmp(0);
    UpdateBlkBmp(0);
    inodes[0] = _inode(0, 0, 0, 0, 0);
    UpdateInode(0);
    WriteDir(".",0,0,0);
    WriteDir("..",1,0,0);
}

void FormatDisk()
{
    memset(sbks.block_bitmap, 0, sizeof(sbks.block_bitmap));
    memset(sbks.inode_bitmap, 0, sizeof(sbks.inode_bitmap));
    memset(inodes, 0, sizeof(inodes));
    //memset(dbks, 0, sizeof(dbks));
    bool _zero = 0;
    FILE *vfs = fopen(filename,"wb");
    for(int i = 0; i < indbmp_size + datablk_size + inodes_size + datablk_size * blkbmp_size; ++i)  {
        //cout << ftell(vfs) << endl;
        fwrite(&_zero, sizeof(_zero), 1, vfs);
    }
    fclose(vfs);
    MakeHome();
    
    
}

int InitBuffer()
{
    FILE *vfs = fopen(filename, "rb+");
    fseek(vfs, 0, SEEK_SET);
    fread(sbks.inode_bitmap, sizeof(bool), sizeof(sbks.inode_bitmap), vfs);
    fread(sbks.block_bitmap, sizeof(bool), sizeof(sbks.block_bitmap), vfs);
    fseek(vfs, InodesPos(0), SEEK_SET);
    fread(inodes, sizeof(_inode), 4096, vfs);
    fclose(vfs);
    return 0;
}




void InitWorkPath()
{
    pathtail = new workdir_pathnode;
    pathtail->dir_inode = 0;
    pathhead = new workdir_pathnode;
    pathhead->dir_inode = 0;
    memset(pathhead->dirname, 0, sizeof(pathhead->dirname));
    memset(pathtail->dirname, 0, sizeof(pathtail->dirname));
    pathhead->prevdir = NULL;
    pathtail->nextdir = NULL;
    pathhead->nextdir = pathtail;
    pathtail->prevdir = pathhead;
    wkpath = pathtail->prevdir;
}


int InitDisk()
{
    FILE *vfs = fopen(filename,"rb");
    if(vfs == NULL) {
        FormatDisk();
    }
	else {
		fclose(vfs);
		InitBuffer();
	}
    InitWorkPath();
    return 1;
}



void GetDirName(int inode_id, int rela_son_id, char* dir_name)
{
    FILE *vfs = fopen(filename, "rb");
    fseek(vfs, DataBlkPos(inodes[inode_id].i_blocks[0]), SEEK_SET);
    fseek(vfs, DirsPos(rela_son_id), SEEK_CUR);
    fread(dir_name, sizeof(char), 252, vfs);
    fclose(vfs);
}



int GetSelfName(int inode_id, char selfname[])
{
    if(inode_id == 0) {
        int namelen = (int) strlen(selfname);
        for(int i = 0; i < namelen; ++i)
            selfname[i] = 0;
        return 0;
    }
    FILE *vfs = fopen(filename, "rb");
    int tempid = -1;
    for(int i = 0; i < 16; ++i) {
        fseek(vfs, DataBlkPos(inodes[inodes[inode_id].fat_id].i_blocks[0]), SEEK_SET);
        fseek(vfs, DirsPos(i), SEEK_CUR);
        fseek(vfs, 252L, SEEK_CUR);
        fread(&tempid, sizeof(int), 1, vfs);
        if(tempid == inode_id) {
            fseek(vfs, DataBlkPos(inodes[inodes[inode_id].fat_id].i_blocks[0]), SEEK_SET);
            fseek(vfs, DirsPos(i), SEEK_CUR);
            fread(selfname, sizeof(char), 252, vfs);
            fclose(vfs);
            return 1;
        }
    }
    fclose(vfs);
    return -1;
}

void NewWorkDirNode(int far_inode_id,int son_inode_id,int rela_son_id = 0)
{
    workdir_pathnode *cur_dirnode = new workdir_pathnode;
    cur_dirnode->dir_inode = son_inode_id;
    GetSelfName(son_inode_id, cur_dirnode->dirname);
    //GetDirName(far_inode_id, rela_son_id, cur_dirnode->dirname);
    cur_dirnode->prevdir = tempwd;
    tempwd->nextdir = cur_dirnode;
    cur_dirnode->nextdir = temptail;
    temptail->prevdir = cur_dirnode;
    tempwd = cur_dirnode;
}



int UpdatePath(char dir_name[], int fat_inode_id, int son_inode_id)
{
    if((strlen(dir_name) == 1 && dir_name[0] == '.') || strlen(dir_name) == 0)
        return 0;
    else if(strlen(dir_name) == 2 && strncmp(dir_name, "..", 2) == 0) {
        if(tempwd->prevdir) {
            tempwd->prevdir->nextdir = temptail;
            temptail->prevdir = tempwd->prevdir;
            delete tempwd;
            tempwd = temptail->prevdir;
        }
    }
    else {
        NewWorkDirNode(fat_inode_id, son_inode_id);
    }
    return 0;
}

int FindPath(char path[], int inode_id,int type_find)
{
    int path_len = (int) strlen(path);
    char SonDirPath[252] = {0};
    int AnoDirPos = 0;
    int son_inode_id = -1, relasondir = -1;
    bool AnotherDir = 0;
    for(int i = 0; i < path_len; ++i)
        if(path[i] == '/') {
            AnotherDir = 1;
            AnoDirPos = i;
            break;
        }
    if(!AnotherDir) {
        if(path_len == 0)
            son_inode_id = inode_id;
        son_inode_id = FindSonPath(path, inode_id, relasondir);
		if (type_find == 1 && son_inode_id != -1 )
            UpdatePath(path, inode_id, son_inode_id);
        return son_inode_id;
    }
    strncpy(SonDirPath, path, AnoDirPos);
    son_inode_id = FindSonPath(SonDirPath, inode_id, relasondir);
    if(son_inode_id == -1)
        return -1;
    if(inodes[son_inode_id].i_mode == 1)
        return -1;
    if(type_find == 1)
        UpdatePath(SonDirPath, inode_id, son_inode_id);
    return FindPath(path + AnoDirPos + 1, son_inode_id, type_find);
}





void FreeDirPath(workdir_pathnode *tempnode, int freemode = 0)
{
    for(workdir_pathnode *tpdir = tempnode; tpdir != NULL; tpdir = tempnode) {
        tempnode = tpdir->nextdir;
        delete tpdir;
    }
    if(freemode == 0) {
        temphead = NULL;
        temptail = NULL;
        tempwd = NULL;
    }
}

void PathError(char path[])
{
    FreeDirPath(temphead);
    cout << path << " " << "No such file or directory" << endl;
    
    //..
}

void CmdError(char cmds[])
{
    FreeDirPath(temphead);
    cout << cmds << ": command not found" << endl;
}

void DirError(char path[])
{
    FreeDirPath(temphead);
    cout << path << " " << "is not a directory." << endl;
}

int GetWorkDir()
{
    return wkpath->dir_inode;
}

int GetFatDir()
{
    return inodes[wkpath->dir_inode].fat_id;
}

/*
int SwitchWorkDir(int status)
{
    switch (status) {
        case 0:
        {
            FreeDirPath(wkpath);
            wkpath = tempwd;
            pathhead = temphead;
            pathtail = temptail;
        }
            break;
        case 1:
        {
            wkpath->nextdir = temphead->nextdir;
            temphead->nextdir->prevdir = wkpath;
            delete pathtail;
            delete temphead;
            wkpath = tempwd;
            pathtail = temptail;
        }
            break;
        case 2:
        {
            if(wkpath->prevdir) {
				wkpath->prevdir->nextdir = temphead->nextdir;
                temphead->nextdir->prevdir = wkpath->prevdir;
            }
            else {
                pathhead->nextdir = temphead->nextdir;
                temphead->nextdir->prevdir = pathhead;
            }
            if(wkpath != pathhead)
				delete wkpath;
            delete pathtail;
            if(temphead->dir_inode != 0)
                wkpath = tempwd;
            else wkpath = pathhead;
            delete temphead;
            pathtail = temptail;
        }
            break;
        default:
            break;
    }
    tempwd = NULL;
    temptail = NULL;
    temphead =NULL;
    return 0;
}*/
int SwitchWorkDir(int switchmode = 0)
{
    switch (switchmode) {
        case 0:
        {
            FreeDirPath(wkpath, 1);
            wkpath = tempwd;
            pathhead = temphead;
            pathtail = temptail;
        }
            break;
        default:
            break;
    }
    tempwd = NULL;
    temptail = NULL;
    temphead =NULL;
    return 0;
}
void CopyPath()
{
    temphead = new workdir_pathnode;
    tempwd = temphead;
    temphead->prevdir = NULL;
    for(workdir_pathnode * ppath = pathhead; ppath != pathtail; ppath = ppath->nextdir) {
        tempwd->dir_inode = ppath->dir_inode;
        strncpy(tempwd->dirname, ppath->dirname, 252);
        tempwd->nextdir = new workdir_pathnode;
        tempwd->nextdir->prevdir = tempwd;
        tempwd = tempwd->nextdir;
    }
    temptail = tempwd;
    temptail->nextdir = NULL;
    temptail->dir_inode = -1;
	tempwd = temptail->prevdir;
    memset(temptail, 0, sizeof(temptail->dirname));
}

void InitTempWD(int initmode = 0)
{
    if(initmode == 1) {
        CopyPath();
    }
    else {
        temptail = new workdir_pathnode;
        temptail->dir_inode = -1;
        temphead = new workdir_pathnode;
        temphead->dir_inode = 0;
        memset(temphead->dirname, 0, sizeof(temphead->dirname));
        memset(temptail->dirname, 0, sizeof(temptail->dirname));
        temphead->prevdir = NULL;
        temptail->nextdir = NULL;
        temphead->nextdir = temptail;
        temptail->prevdir = temphead;
        tempwd = temptail->prevdir;
    }
}

int GetAboPath(char *path)
{
    int pathlen = (int) strlen(path), dir_name_len = 0;
    memset(path, 0, pathlen);
    if(pathhead->nextdir == pathtail)
        path[0] = '/';
    else for(workdir_pathnode* itdir = pathhead; itdir->nextdir != NULL; itdir = itdir->nextdir) {
        dir_name_len = (int) strlen(itdir->dirname);
        if(itdir == wkpath) {
            sprintf(path, "%s", itdir->dirname);
            path += dir_name_len;
        }
        else {
            sprintf(path, "%s/", itdir->dirname);
            path += dir_name_len + 1;
        }
        
    }
    return 0;
}

int PrintWorkPath()
{
    if(pathhead->nextdir == pathtail)
        cout << '/';
    else for(workdir_pathnode* itdir = pathhead; itdir->nextdir != NULL; itdir = itdir->nextdir)
            if(itdir == wkpath)
                cout <<itdir->dirname;
            else cout << itdir->dirname << '/';
    cout << endl;
    return 0;
}


int GetPathInode(char path[], int type_judge)
{
    //int path_len = (int) strlen(path);
    int nextdirpos = 0;
    int src_inode = 0, initpathmode = 0;
    int SonDirStatus = 0;
    if(path[0] == '/') {
        src_inode = 0;
        nextdirpos = 1;
        initpathmode = 0;
    }
    else {
        initpathmode = 1;
        src_inode = GetWorkDir();
        SonDirStatus = 1;
        nextdirpos = 0;
    }
    if(src_inode == -1) {
        //PathError(path);
        return -1;
    }
    if(type_judge == 1)
        InitTempWD(initpathmode);
    int dst_inode_id = FindPath(path + nextdirpos, src_inode, type_judge);
    if(dst_inode_id == -1) {
        // PathError(path);
        return -1;
    }
    if(type_judge == 1 && inodes[dst_inode_id].i_mode == 0)
        SwitchWorkDir();
    return dst_inode_id;
}
int ChangeDir(char *path)
{
    
    int returnstatus = GetPathInode(path, 1);
    if(returnstatus == -1)
        PathError(path);
    else if(inodes[returnstatus].i_mode == 1)
        DirError(path);
    return returnstatus;
}

//返回10代表已存在
int ReName(char path[], char AimedName[])
{
    int srcInodeId = GetPathInode(path);
    char SonName[252] = {0}, zero_str[252] = {0};
    int TempInode = GetPathInode(AimedName);
    if(srcInodeId < 0)
        return srcInodeId;
    else {
        if(TempInode != srcInodeId && TempInode > 0)
            return 10;
        GetSelfName(srcInodeId, SonName);
        int fat_inodeid = inodes[srcInodeId].fat_id, rela_id = -1;
        FindSonPath(SonName, fat_inodeid, rela_id);
        WriteDir(zero_str, rela_id, inodes[fat_inodeid].i_blocks[0], srcInodeId);
        WriteDir(AimedName, rela_id, inodes[fat_inodeid].i_blocks[0], srcInodeId);
    }
    //char current_path[input_buffer_length] = {0};
   // GetAboPath(current_path);
    //GetPathInode(".", 1);
    // update son fat
    return 0;
}

void NoExistedErr(char path[])
{
    cout << path << " No such file or directory." << endl;
}

int mv(char path[], char AimedName[])
{
    int tempstatus = ReName(path, AimedName);
    if(tempstatus < 0) {
        NoExistedErr(path);
        return -11;
    }
    else if(tempstatus == 10) {
        ExistedError(AimedName);
        return 10;
    }
    return 0;
    
}


int ListDirs(char path[])
{
    FILE *vfs = fopen(filename, "rb");
    cout.setf(ios::left);
    char dir_name[253] = {0};
    int tar_inodeid = 0;
    if(strlen(path) == 0)
        tar_inodeid = wkpath->dir_inode;
    else tar_inodeid = GetPathInode(path);
    int dir_entry_id = -1, son_cnt = 0;
    if(inodes[tar_inodeid].i_mode == 1) {
        GetSelfName(tar_inodeid, dir_name);
        cout << dir_name;
    }
    else for(int i = 2; i < 16; ++i) {
            fseek(vfs, DataBlkPos(inodes[tar_inodeid].i_blocks[0]), SEEK_SET);
            fseek(vfs, DirsPos(i), SEEK_CUR);
            memset(dir_name, 0, sizeof(dir_name));
            fread(dir_name, sizeof(char), 252, vfs);
            fread(&dir_entry_id, sizeof(int), 1, vfs);
            if(dir_entry_id != 0) {
                if(son_cnt > 1 && (son_cnt) % 5 == 0)
                    cout << endl;
                cout.width(10);
                cout << dir_name << "   ";
                son_cnt++;
                
            }
        }
    cout << endl;
    fclose(vfs);
    return 0;
}

int InitMv(char path[], char *inputcontent)
{
    int pathlen = (int) strlen(path), spacepos = -1;
    for(int i = 0; i < pathlen - 1; ++i)
        if(path[i] == ' ') {
            spacepos = i;
            strncpy(inputcontent, path + spacepos + 1, pathlen - spacepos - 1);
            break;
        }
    return spacepos;
}

int InitEcho(char path[], char *inputcontent)
{
    int pathlen = (int) strlen(path), spacepos = -1;
    for(int i = 0; i < pathlen - 1; ++i)
        if(path[i] == ' ') {
            spacepos = i;
            strncpy(inputcontent, path, i);
            break;
    }
    return spacepos;
}

bool IsExit(int flag)
{
    return flag == EXITFLAG;
}

char tempcmd[input_buffer_length] = {0};
bool IsCmdErr(const char* cmd, char path[], int judgemode = 1)
{
    memset(tempcmd, 0, sizeof(tempcmd));
    int cmdlen = (int) strlen(cmd), pathlen = (int) strlen(path);
    if(pathlen >= cmdlen) {
        if(pathlen == cmdlen && strncmp(cmd, path, pathlen) == 0)
            return 0;
        else if(judgemode == 1){
            strncpy(tempcmd, cmd, cmdlen);
            tempcmd[cmdlen] = ' ';
            if(strncmp(tempcmd, path, cmdlen+1) == 0)
                return 0;
        }
    }
    return 1;
}

int WaitMessage()
{
    cout << ">> " ;
    memset(inputbuffer, 0, sizeof(inputbuffer));
   // cin >> inputbuffer;
    string input_str;
    getline(cin, input_str);
	//cin.getline(inputbuffer, input_buffer_length);
    if(input_str.length() >= input_buffer_length) {
        cout << input_str << " : The path is too long" << endl;
        return 9;
    }
    strncpy(inputbuffer, input_str.c_str(), input_str.length());
    //cout << endl;
    int inputlen = (int) strlen(inputbuffer), echopos = -1;
    switch (inputbuffer[0]) {
        case 'c':
        {
            
            if(inputlen > 1 && inputbuffer[1] == 'd') {
                if(!IsCmdErr("cd", inputbuffer)) {
                    if(!((inputlen > 3) && (strncmp(inputbuffer, "cd ", 3) == 0))) {
                        PathError(inputbuffer + 3);
                        return 1;
                    }
                //strcpy(dirpath, inputbuffer + 3);
                    else ChangeDir(inputbuffer + 3);
                    
                }
            }
            else if(!IsCmdErr("cat", inputbuffer)) {
                    if(!(inputlen > 4 && strncmp(inputbuffer, "cat ", 4) == 0)) {
                        PathError(inputbuffer + 4);
                        return 1;
                    }
                    else
                        return cat(inputbuffer + 4);
            }
            else {
                CmdError(inputbuffer);
                return 2;
            }
        }
            break;
        case 'p':
        {
            if(IsCmdErr("pwd", inputbuffer, 0)) {
                CmdError(inputbuffer);
                return 2;
            }
            else PrintWorkPath();
        }
            break;
        case 'l':
        {
            if(IsCmdErr("ls", inputbuffer)) {
                CmdError(inputbuffer);
                return 2;
            }
            return ListDirs(inputbuffer + 3);
        }
            break;
        case 'm':
        {
            if(inputlen > 2 && inputbuffer[1] == 'k') {
                if(IsCmdErr("mkdir", inputbuffer)) {
                    CmdError(inputbuffer);
                    return 2;
                }
                else if(!(inputlen > 6 && strncmp(inputbuffer, "mkdir ", 6) == 0)) {
                    PathError(inputbuffer + 6);
                    return 1;
                }
                return MakeDir(inputbuffer + 6);
            }
            else {
                if(IsCmdErr("mv", inputbuffer)) {
                    CmdError(inputbuffer);
                    return 2;
                }
                else {
                    memset(inputcontent, 0, sizeof(inputcontent));
                    int SpacePos = InitMv(inputbuffer + 3, inputcontent);
                    if(SpacePos == -1) {
                        PathError(inputbuffer);
                        return 1;
                    }
                    else {
                        memset(inputbuffer + SpacePos + 3, 0, inputlen - SpacePos - 4);
                        return mv(inputbuffer + 3, inputcontent);
                    }
                    
                }
            }
        }
            break;
        case 'e':
        {
            if(inputlen > 2 && inputbuffer[1] == 'c') {
                if(!IsCmdErr("echo", inputbuffer)) {
                    if(!(inputlen > 6) && strncmp(inputbuffer, "echo ", 5) == 0) {
                        PathError(inputbuffer + 5);
                        return 1;
                    }
                    for(int i = 6; i < inputlen; ++i)
                        if(inputbuffer[i] == ' '){
                            memset(inputcontent, 0 ,sizeof(inputcontent));
							//cout << inputbuffer + 5 << endl;
                            echopos = InitEcho(inputbuffer + 5, inputcontent);
                            int input_cnt_len = (int)strlen(inputcontent);
                            if(echopos == -1) {
                                PathError(inputbuffer+5);
                                return 1;
                            }
                            if(input_cnt_len > 4096) {
                                cout << "The text is too long !" << endl;
                                return 7;
                            }
							return echo(inputbuffer + 6 + echopos, inputcontent);
                    }
                }
            }
            else if(inputlen == 4 && strcpy(inputbuffer, "exit"))
                return EXITFLAG;
            else {
                CmdError(inputbuffer);
                return 2;
                }
        }
            break;
        case 'r':
        {
            if(!IsCmdErr("rm", inputbuffer)) {
                if(inputlen > 3 && strncmp(inputbuffer, "rm ", 3) == 0) {
                    return rm(inputbuffer +3);
                }
                else {
                    PathError(inputbuffer + 3);
                    return 1;
                }
            }
            else if(!IsCmdErr("rmdir", inputbuffer)) {
                    if(!(inputlen > 6) && strncmp(inputbuffer, "rmdir ", 6) == 0) {
                        PathError(inputbuffer + 6);
                        return 1;
                }
                else return rmdir(inputbuffer + 6);
            }
            else {
                CmdError(inputbuffer);
                return 2;
            }
        }
            break;
        default:
            CmdError(inputbuffer);
            return 2;
            break;
    }
    
    return 0;
}

void debug()
{
    cout << "" << "ha" << endl;
}
