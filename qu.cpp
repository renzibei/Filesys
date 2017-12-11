#include "qu.h"
const char filename[12] = "Filesys.vfs";
const int inode_size = 32, datablk_size = 4096, dir_size = 256;
const int indbmp_size = 4096, blkbmp_size = 4096, inodes_size = inode_size * 4096;

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

void WriteDir(const char *dir_name, int dir_id, int inode_id)
{
    FILE *vfs = fopen(filename,"rb+");
    fseek(vfs,DataBlkPos(inode_id),SEEK_SET);
    fseek(vfs,DirsPos(dir_id),SEEK_CUR);
    fwrite(dir_name,sizeof(char), strlen(dir_name), vfs);
    fseek(vfs,DataBlkPos(inode_id),SEEK_SET);
    fseek(vfs,252,SEEK_CUR);
    fwrite(&inode_id, sizeof(int), 1, vfs);
    fclose(vfs);
}

int UpdateInode(int x)
{
    FILE *vfs = fopen(filename,"rb+");
    fseek(vfs, InodesPos(x), SEEK_SET);
    fwrite(&inodes[x].i_id,sizeof(int), 3, vfs);
    fwrite(&inodes[x].i_blocks,sizeof(int),1,vfs);
    fwrite(inodes[x].i_place_holder,sizeof(char),PLACEHOLDER,vfs);
    fclose(vfs);
    return 0;
}

int MakeDir(char path[])
{
    int path_len = (int) strlen(path), divpos = -1;
    for(int i = path[path_len-1]; i > -1; --i)
        if(path[i] == '/') {
            divpos = i;
            break;
        }
    if(divpos == -1)
        
}

void MakeHome()
{
    
    sbks.inode_bitmap[0] = 1;
    sbks.block_bitmap[0] = 1;
    UpdateIndBmp(0);
    UpdateBlkBmp(0);
    inodes[0].i_id = 0;
    inodes[0].i_mode = 0;
    inodes[0].i_blocks[0] = 0;
    UpdateInode(0);
    WriteDir(".",0,0);
    WriteDir("..",1,0);
    
    
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
    fread(&inodes, sizeof(_inode), 4096, vfs);
    fclose(vfs);
    return 0;
}

void InitWorPath()
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
    return 1;
}

int FindSonPath(char sonpath[],int inode_id, int &relasondir)
{
    FILE *vfs = fopen(filename, "rb");
    int dst_inode_id = -1;
    char pathname[252] = {0};
    for(int i = 0; i < 16; ++i) {
        fseek(vfs,DataBlkPos(inodes[inode_id].i_blocks[0]),SEEK_SET);
        fseek(vfs,DirsPos(i),SEEK_CUR);
        fread(pathname, sizeof(char), 252, vfs);
        if(strcmp(sonpath, pathname) == 0) {
            fread(&dst_inode_id, sizeof(int), 1, vfs);
            return dst_inode_id;
        }
    }
    fclose(vfs);
    return -1;
}

void GetDirName(int inode_id, int rela_son_id, char* dir_name)
{
    FILE *vfs = fopen(filename, "rb");
    fseek(vfs, DataBlkPos(inode_id), SEEK_SET);
    fseek(vfs, DirsPos(rela_son_id), SEEK_CUR);
    fread(dir_name, sizeof(char), 252, vfs);
    fclose(vfs);
}

void NewWorkDirNode(int far_inode_id,int son_inode_id,int rela_son_id)
{
    workdir_pathnode *cur_dirnode = new workdir_pathnode;
    cur_dirnode->dir_inode = son_inode_id;
    GetDirName(far_inode_id, rela_son_id, cur_dirnode->dirname);
    cur_dirnode->prevdir = tempwd;
    tempwd->nextdir = cur_dirnode;
    cur_dirnode->nextdir = temptail;
    temptail->prevdir = cur_dirnode;
    tempwd = cur_dirnode;
}

int FindPath(char path[], int inode_id,int type_find = 0)
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
        son_inode_id = FindSonPath(path, inode_id, relasondir);
        return son_inode_id;
    }
    strncpy(SonDirPath, path, AnoDirPos + 1);
    son_inode_id = FindSonPath(SonDirPath, inode_id, relasondir);
    if(son_inode_id == -1)
        return -1;
    if(type_find == 1)
        NewWorkDirNode(inode_id, son_inode_id, relasondir);
    //memset(SonDirPath, 0, sizeof(SonDirPath));
   // strcpy(SonDirPath, path + AnoDirPos + 1);
    return FindPath(path + AnoDirPos + 1, son_inode_id);
}

int FindFileInDir



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
    for(workdir_pathnode* itdir = pathhead; itdir != NULL; itdir = itdir->nextdir) {
        cout << itdir->dirname << '/';
    }
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
int GetPathInode(char path[], int type_judge = 0) // 要改改
{
    int path_len = (int) strlen(path)， nextdirpos = 0;
    int src_inode = 0;
    int SonDirStatus = 0;
    if(path[0] == '/')
        src_inode = 0, nextdirpos = 1;
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
    else src_inode = -1;
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
    else if(nodes[returnstatus].i_mode == 1)
        DirError(path);
    return returnstatus;
}

int ListDirs(char path[])
{
    return 0;
}

int WaitMessage()
{
    cout << ">> " ;
    memset(inputbuffer, 0, sizeof(inputbuffer));
    cin >> inputbuffer;
    char dirpath[input_buffer_length] = {0};
    switch (inputbuffer[0]) {
        case 'c':
        {
            if(inputbuffer[1] != 'd') {
                PathError(inputbuffer);
                return 1;
            }
            strcpy(dirpath, inputbuffer + 3);
            ChangeDir(dirpath);
        }
            break;
        case 'p':
        {
            if(strncmp(inputbuffer,"pwd",3) != 0 ) {
                PathError(inputbuffer);
                return 1;
            }
            else PrintWorkPath();
        }
            break;
        case 'l':
        {
            if(strncmp(inputbuffer, "ls", 2) != 0) {
                PathError(inputbuffer);
                return 1;
            }
            strcpy(dirpath, inputbuffer + 3);
            ListDirs(dirpath);
        }
            break;
        case 'm':
        {
            if(strncmp(inputbuffer, "mkdir", 5) != 0) {
                PathError(inputbuffer);
                return 1;
            }
            strcpy(dirpath, inputbuffer + 6);
            MakeDir(dirpath);
        }
            break;
        default:
            break;
    }
    
    return 0;
}
