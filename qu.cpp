#include "qu.h"
#define EXITFLAG 0x7fffffff
const char filename[12] = "Filesys.vfs";
const int inode_size = 32, datablk_size = 4096, dir_size = 256;
const int indbmp_size = 4096, blkbmp_size = 4096, inodes_size = inode_size * 4096;
char inputbuffer[input_buffer_length];

workdir_pathnode *pathhead = NULL;
workdir_pathnode *wkpath   = NULL;
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
    FILE *vfs = fopen(filename,"rb+");
    fseek(vfs,DataBlkPos(dir_block_id),SEEK_SET);
    fseek(vfs,DirsPos(relative_dir_entry_id),SEEK_CUR);
    fwrite(dir_name,sizeof(char), strlen(dir_name), vfs);
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
            return dst_inode_id;
        }
        fread(&temp_inode_id, sizeof(int), 1, vfs);
        if(temp_inode_id == 0 && !existedfreeentry) {
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



int MakeDir(char path[]) //没写完接着写
{
    int path_len = (int) strlen(path), divpos = -1, fat_inode = 0;
    char fat_path[input_buffer_length] = {0}, dir_name[253] = {0};
    for(int i = path[path_len-1]; i > -1; --i)
        if(path[i] == '/') {
            divpos = i;
            break;
        }
    if(divpos == -1)
        fat_inode = GetWorkDir();
    else {
        strncpy(fat_path, path, divpos + 1);
        fat_inode = GetPathInode(fat_path);
        if(fat_inode == -1) {
            PathError(fat_path);
            return -1;
        }
    }
    int rela_id = -1;
    if(FindSonPath(dir_name, fat_inode, rela_id) != -1) {
        ExistedError(path);
        return -2;
    }
    if(rela_id == -1) {
        cout << "The Directory is full!" << endl;
        return -3;
    }
    int new_dir_inode = find_free_blkbmp();
    sbks.inode_bitmap[new_dir_inode] = 1;
    UpdateInode(new_dir_inode);
    int new_blk_id = find_free_blkbmp();
    inodes[new_dir_inode] = _inode(new_dir_inode, 0, 0, fat_inode, new_blk_id);
    UpdateInode(new_dir_inode);
    sbks.inode_bitmap[new_blk_id] = 1;
    UpdateBlkBmp(new_blk_id);
    WriteDir(dir_name, rela_id, fat_inode, new_blk_id);
    WriteDir(".", 0, new_blk_id, new_blk_id);
    WriteDir("..", 1, new_blk_id, fat_inode);
    return 0;
        
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
    pathtail->dir_inode = -1;
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
        fclose(vfs);
        FormatDisk();
        return 0;
    }
    else InitBuffer();
    InitWorkPath();
    return 1;
}



void GetDirName(int inode_id, int rela_son_id, char* dir_name)
{
    FILE *vfs = fopen(filename, "rb");
    fseek(vfs, DataBlkPos(inode_id), SEEK_SET);
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
        fseek(vfs, DataBlkPos(inodes[inode_id].fat_id), SEEK_SET);
        fseek(vfs, DirsPos(i), SEEK_CUR);
        fseek(vfs, 252L, SEEK_CUR);
        fread(&tempid, sizeof(int), 1, vfs);
        if(tempid == inode_id) {
            fseek(vfs, DataBlkPos(inode_id), SEEK_SET);
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
        if(type_find == 1)
            NewWorkDirNode(inode_id, son_inode_id, relasondir);
        son_inode_id = FindSonPath(path, inode_id, relasondir);
        return son_inode_id;
    }
    strncpy(SonDirPath, path, AnoDirPos + 1);
    son_inode_id = FindSonPath(SonDirPath, inode_id, relasondir);
    if(son_inode_id == -1)
        return -1;
    if(type_find == 1)
        NewWorkDirNode(inode_id, son_inode_id, relasondir);
    return FindPath(path + AnoDirPos + 1, son_inode_id, type_find);
}





void FreeDirPath(workdir_pathnode *tempnode)
{
    for(workdir_pathnode *tpdir = tempnode; tpdir != NULL; tpdir = tempnode) {
        tempnode = tpdir->nextdir;
        delete tpdir;
    }
    
}

void PathError(char path[])
{
    FreeDirPath(temphead);
    cout << path << " " << "No such file or directory" << endl;
    
    //..
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
    return wkpath->prevdir->dir_inode;
}

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
            temphead->nextdir->prevdir = wkpath->nextdir;
            delete pathtail;
            delete temphead;
            pathtail = temptail;
        }
            break;
        case 2:
        {
            wkpath->prevdir->nextdir = temphead->nextdir;
            temphead->nextdir->prevdir = wkpath->prevdir;
            delete wkpath;
            delete pathtail;
            delete temphead;
            wkpath = tempwd;
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

void InitTempWD()
{
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

int PrintWorkPath()
{
    for(workdir_pathnode* itdir = pathhead; itdir != NULL; itdir = itdir->nextdir)
        cout << itdir->dirname << '/';
    cout << endl;
    return 0;
}
/*
//获得文件夹的inode,文件路径错误时返回值为-1，是文件而不是文件夹时返回-2
int GetDirPathInode(char path[], int type_judge = 0)  //type_judge == 0时是正常的获得文件夹的inode 普通调用时可忽略这一参数
{
    int path_len = (int) strlen(path);
    int src_inode = 0;
    int SonDirStatus = 0;
    if(path[0] == '/')
        src_inode = 0;
    else if(path[0] == '.') {
        if(path_len ==2 && path[1] == '.') {
            src_inode = GetFatDir();
            SonDirStatus = 2;
        }
        else if(path_len == 1) {
            src_inode = GetWorkDir();
            SonDirStatus = 1;
        }
    }
    else src_inode = -1;
    if(src_inode == -1) {
        //PathError(path);
        return -1;
    }
    if(type_judge == 1)
        InitTempWD();
    int dst_inode_id = FindPath(path, src_inode);
    if(dst_inode_id == -1) {
       // PathError(path);
        return -1;
    }
    
    if(inodes[dst_inode_id].i_mode == 1) {
        //DirError(path)
        return -2;
    }
    if(type_judge == 1)
        SwitchWorkDir(SonDirStatus);
    return 0;
}
*/
//直接查找path[]对应的文件或文件夹，返回inode_id，错误返回-1
int GetPathInode(char path[], int type_judge)
{
    int path_len = (int) strlen(path);
    int nextdirpos = 0;
    int src_inode = 0;
    int SonDirStatus = 0;
    if(path[0] == '/') {
        src_inode = 0;
        nextdirpos = 1;
    }
    else if(path[0] == '.' && path_len > 1) {
        if(path[1] == '/') {
            src_inode = GetWorkDir();
            SonDirStatus = 1;
            nextdirpos = 2;
        }
        else if(path_len > 2 && path[1] == '.' && path[2] == '/') {
            src_inode = GetFatDir();
            SonDirStatus = 2;
            nextdirpos = 3;
        }
    }
    else {
        src_inode = GetWorkDir();
        SonDirStatus = 1;
        nextdirpos = 2;
    }
    if(src_inode == -1) {
        //PathError(path);
        return -1;
    }
    if(type_judge == 1)
        InitTempWD();
    int dst_inode_id = FindPath(path + nextdirpos, src_inode);
    if(dst_inode_id == -1) {
        // PathError(path);
        return -1;
    }
    /*
    if(inodes[dst_inode_id].i_mode == 1) {
        //DirError(path)
        return -2;
    }*/
    if(type_judge == 1 && inodes[dst_inode_id].i_mode == 0)
        SwitchWorkDir(SonDirStatus);
    return dst_inode_id;
}

int ChangeDir(char path[])
{
    /*
    int path_len = (int) strlen(path);
    int src_inode = 0;
    int SonDirStatus = 0;
    if(path[0] == '/')
        src_inode = 0;
    else if(path[0] == '.') {
        if(path_len ==2 && path[1] == '.') {
            src_inode = GetFatDir();
            SonDirStatus = 2;
        }
        else if(path_len == 1) {
            src_inode = GetWorkDir();
            SonDirStatus = 1;
        }
    }
    else src_inode = -1;
    if(src_inode == -1) {
        PathError(path);
        return -1;
    }
    InitTempWD();
    int dst_inode_id = FindPath(path, src_inode);
    if(dst_inode_id == -1) {
        PathError(path);
        return -1;
    }
    
    if(inodes[dst_inode_id].i_mode == 1) {
        DirError(path);
        return -1;
    }
    SwitchWorkDir(SonDirStatus); */
    int returnstatus = GetPathInode(path, 1);
    if(returnstatus == -1)
        PathError(path);
    else if(inodes[returnstatus].i_mode == 1)
        DirError(path);
    return returnstatus;
}

int ListDirs(char path[])
{
    FILE *vfs = fopen(filename, "rb");
    char dir_name[253] = {0};
    for(int i = 0; i < 16; ++i) {
        fseek(vfs, DataBlkPos(wkpath->dir_inode), SEEK_SET);
        fseek(vfs, DirsPos(i), SEEK_CUR);
        memset(dir_name, 0, sizeof(dir_name));
        fread(dir_name, sizeof(char), 252, vfs);
        cout << " ";
    }
    fclose(vfs);
    return 0;
}

int InitEcho(char path[], char *inputcontent)
{
    int pathlen = (int) strlen(path), spacepos = -1;
    for(int i = 0; i < pathlen - 1; ++i)
        if(path[i] == ' ') {
            spacepos = i;
            strncpy(inputcontent, path, i + 1);
            break;
    }
    return spacepos;
}

bool IsExit(int flag)
{
    return flag == EXITFLAG;
}

int WaitMessage()
{
    cout << ">> " ;
    memset(inputbuffer, 0, sizeof(inputbuffer));
    cin >> inputbuffer;
    cout << endl;
    char inputcontent[input_buffer_length] = {0};
    int inputlen = (int) strlen(inputbuffer), echopos = -1;
    switch (inputbuffer[0]) {
        case 'c':
        {
            if(inputlen > 2 && inputbuffer[1] == 'd') {
                if(!((inputlen > 3) && (strncmp(inputbuffer, "cd ", 3) == 0))) {
                    PathError(inputbuffer);
                    return 1;
                }
                //strcpy(dirpath, inputbuffer + 3);
                ChangeDir(inputbuffer + 3);
            }
            else {
                if(!(inputlen > 4 && strncmp(inputbuffer, "cat ", 4))) {
                    PathError(inputbuffer);
                    return 1;
                }
                else {
                    return cat(inputbuffer + 4);
                }
            }
        }
            break;
        case 'p':
        {
            if(!(inputlen == 3 && strncmp(inputbuffer,"pwd", 3) == 0 )) {
                PathError(inputbuffer);
                return 1;
            }
            else PrintWorkPath();
        }
            break;
        case 'l':
        {
            if(!( inputlen == 2 && strncmp(inputbuffer, "ls", 2) == 0)) {
                PathError(inputbuffer);
                return 1;
            }
            //strcpy(dirpath, inputbuffer + 3);
            ListDirs(inputbuffer + 3);
        }
            break;
        case 'm':
        {
            if(!(inputlen > 6 && strncmp(inputbuffer, "mkdir", 5) == 0)) {
                PathError(inputbuffer);
                return 1;
            }
            //strcpy(dirpath, inputbuffer + 6);
            MakeDir(inputbuffer + 6);
        }
            break;
        case 'e':
        {
            if(inputlen > 2 && inputbuffer[1] == 'c') {
                if(!(inputlen > 6) && strncmp(inputbuffer, "echo ", 5)) {
                    PathError(inputbuffer);
                    return 1;
                }
                for(int i = 6; i < inputlen; ++i)
                    if(inputbuffer[i] == ' '){
                        memset(inputcontent, 0 ,sizeof(inputcontent));
                        echopos = InitEcho(inputbuffer + 6, inputcontent);
                        if(echopos == -1) {
                            PathError(inputbuffer);
                            return 1;
                        }
                        return echo(inputbuffer + 6 + echopos, inputcontent);
                }
            }
            else if(inputlen == 4 && strcpy(inputbuffer, "exit"))
                return EXITFLAG;
            else {
                PathError(inputbuffer);
                return 1;
                }
        }
            break;
        case 'r':
        {
            if(inputlen > 3 && strncmp(inputbuffer, "rm ", 3)) {
                return rm(inputbuffer +3);
            }
            else if(!(inputlen > 6) && strncmp(inputbuffer, "rmdir ", 6)) {
                PathError(inputbuffer);
                return 1;
            }
            else return rmdir(inputbuffer + 6);
        }
            break;
        default:
            PathError(inputbuffer);
            return 1;
            break;
    }
    
    return 0;
}
