#include "qu.h"

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
    fseek(vfs,DirsPos(252),SEEK_CUR);
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
    //char point[2] = ".";
    /*fseek(vfs,DataBlkPos(0),SEEK_SET);
    fseek(vfs,DirsPos(0),SEEK_CUR);
    char points[3] = "..";
    fwrite(points, sizeof(char), 1, vfs);
    fseek(vfs,DataBlkPos(0),SEEK_SET);
    fseek(vfs,DirsPos(252),SEEK_CUR);
    int zero = 0;
    fwrite(&zero, sizeof(int), 1, vfs); */
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
    for(int i = 0; i < 16520; ++i)
        fwrite(&_zero, sizeof(_zero), 1, vfs);
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

