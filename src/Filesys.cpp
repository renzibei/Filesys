//
//  main.cpp
//  
//
//  Created by Fan Qu on 2017/11/30.
//


#define _CRT_SECURE_NO_WARNINGS
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
