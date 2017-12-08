# 数据结构
## 基础
    super_group
    inode
    file_block
    dir_entry
    dir_block
## 增加
    inodestack 空余索引栈
    blockstack 空余数据块栈
    
    
# 文件
## 名称
    Filesys.vfs
## 大小
    16520KB
## 分布
    inode bitmap 1-4KB
    block bitmap 5-8KB
    inode_0~4095 9-136KB
    block_0~4095 137-16520KB
    

# 函数
## 基础
    pwd
    cd
    mkdir
    ls
    rmdir
    echo
    cat
    rm
## 成员函数
    ClearDisk 格式化磁盘
    FindFile 寻找文件并判断
## 增加
    touch <file> //新建文件
    add <file> //向文件尾添加
    
