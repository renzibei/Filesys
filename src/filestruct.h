#ifndef FILESTRUCT_H
#define FILESTRUCT_H
#include "Filesys.h"

#ifndef PLACEHOLDER
#define PLACEHOLDER 12
#endif

extern const char filename[12];
extern const int inode_size,datablk_size, dir_size, indbmp_size, blkbmp_size, inodes_size;

struct _super_block {
    bool inode_bitmap[4096];
    bool block_bitmap[4096];
};

struct _inode {
    int i_id;
    int i_mode; // 0代表文件夹，1代表文件
    int i_file_size;
    int fat_id; //父文件夹的inode_id,没有则为0
    //...
    int i_blocks[1]; //都是从0开始数
    char i_place_holder[PLACEHOLDER]; //占位符
    _inode () {}
    _inode(int _i_id, int _i_imode, int _i_i_fs, int _fat_id, int _blocks)
    {
        i_id = _i_id;
        i_mode = _i_imode;
        i_file_size = _i_i_fs;
        fat_id = _fat_id;
        i_blocks[0] = _blocks;
        for(int i = 0; i < PLACEHOLDER; ++i)
            i_place_holder[i] = 0;
    }
};

//文件对应数据块
struct _file_block {
    char data[4096];
};

// dir_entry
struct dir_entry {
    char name[252];
    int inode_id;
};

struct _dir_block {
    dir_entry dirs[16];
};
/*
struct _data_block {
    _file_block *file_block;
    _dir_block *dir_block;
} dbks[4096];*/

#endif
