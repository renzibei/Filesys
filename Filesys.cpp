//
//  main.cpp
//  
//
//  Created by Fan Qu on 2017/11/30.
//


#include "Filesys.h"
_super_block sbks;
_inode inodes[4096];

int main()
{
    InitDisk();
    for(;;) {
        if(IsExit(WaitMessage()))
            break;
    }
    return 0;
}
