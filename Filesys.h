//
//  Filesys.h
//  Filesys_xcode
//
//  Created by Fan Qu on 2017/12/8.
//  Copyright © 2017年 GF. All rights reserved.
//

#ifndef Filesys_h
#define Filesys_h
#include "qu.h"
#include "fu.h"
#include "filestruct.h"
#include <iostream>
#include <cstdio>
#include <cstring>

#define input_buffer_length 4096

extern _inode inodes[4096];
extern _super_block sbks;

char inputbuffer[input_buffer_length];

using namespace std;


#endif /* Filesys_h */
