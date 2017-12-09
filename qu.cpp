#include "qu.h"

int UpdateIndBmp(int x)
{
    FILE *vfs = fopen(filename, "rb+");
    fseek(vfs, x, SEEK_SET);
    fwrite(&sbks.inode_bitmap[x], sizeof(bool), 1, vfs);
    fclose(vfs);
}

int UpdateBlkBmp(int x)
{
    FILE *vfs = fopen(filename, "rb+");
    fseek(vfs, x, SEEK_SET);
    fwrite(&sbks.block_bitmap[x], sizeof(bool), 1, vfs);
    fclose(vfs);
}

void FormatDisk()
{
    memset(sbks.block_bitmap, 0, sizeof(sbks.block_bitmap));
    memset(sbks.inode_bitmap, 0, sizeof(sbks.inode_bitmap));
    memset(inodes, 0, sizeof(inodes));
    memset(dbks, 0, sizeof(dbks));
    FILE *vfs = fopen(filename,"wb");
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

