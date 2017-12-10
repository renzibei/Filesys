#ifndef FILESTRUCT_H
#define FILESTRUCT_H

const char filename[12] = "Filesys.vfs";

struct _super_block {
    bool inode_bitmap[4096];
    bool block_bitmap[4096];
} sbks;

struct _inode {
    int i_id;
    int i_mode; // 0代表文件夹，1代表文件
    int i_file_size;
    //...
    int i_blocks[1];
    char i_place_holder[16]; //占位符
} inodes[4096];

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
