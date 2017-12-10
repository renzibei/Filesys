#include "qu.h"

long DataBlkPos(int x) //返回第x个data_block在磁盘文件中的位置
{
    return indbmp_size + blkbmp_size + inodes_size + datablk_size * x;
}

long InodesPos(int x) //返回第x个inode在磁盘文件中的位置
{
    return indbmp_size + blkbmp_size + inode_size * x;
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

void MakeHome(FILE *vfs)
{
    sbks.inode_bitmap[0] = 1;
    sbks.block_bitmap[0] = 1;
    inodes[0].i_id = 0;
    inodes[0].i_mode = 0;
    inodes[0].i_blocks[0] = 0;
    fseek(vfs,DataBlkPos(0),SEEK_SET);
    
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
    MakeHome(vfs);
    fclose(vfs);
    
}

int InitDisk()
{
    FILE *vfs = fopen(filename,"rb");
    if(vfs == NULL) {
        fclose(vfs);
        FormatDisk();
        return 0;
    }
    return 1;
}

