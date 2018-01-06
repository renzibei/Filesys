#
# 模拟文件系统说明

作者：傅舟涛 瞿凡

## 一、系统概述

程序将用&quot;Filesys.vfs&quot;模拟磁盘，首次运行时会创建磁盘文件并初始化，请注意， **初始化过程中请勿关闭程序** 。

## 二、名词解释

### 2.1、路径

路径是用户在磁盘上寻找文件或目录时，所历经的文件线路，用&quot;/&quot;分隔不同的名称。路径分为绝对路径和相对路径。特别的，每个文件夹下都有&quot;.&quot;和&quot;..&quot;两个文件夹，分别表示自身以及它的上级文件夹。根目录的上级文件夹为它自身。

#### 2.1.1相对路径

相对路径，指的是从当前目录开始寻找的路径，格式为&quot;a/b/c&quot;，表示的是当前目录下的a目录下的b目录下的c目录或文件。

#### 2.1.2绝对路径

绝对路径，指的是从根目录开始寻找的路径，格式为&quot;/a/b/c&quot;，表示的是根目录下的a目录下的b目录下的c目录或文件。特别的， **&quot;/&quot;表示根目录** 。

### 2.2、长度

本说明中，长度指的是不包含&#39;\0&#39;的char的个数，或者说是字节数。简单来说，长度指的是有多少个字符（一个中文算两个字符，但本程序禁止使用中文）， **如无特殊说明，输入的$path长度不超过1036035。**

### 2.3、提示

指的是程序输出的内容。提示&quot;&gt;&gt; &quot;表示可以继续输入，除格式化有确认操作提示外，其余若操作成功均无提示，若操作失败会提示失败原因。

### 2.4、变量与符号$

任何形如&quot;$x&quot;的形式的表示一个变量，通常为输入或输入相关量。常见的有$path（表示文件或目录路径）、$str（表示字符串）、$mode（表示refresh函数的操作种类（见3.5.1））。

### 2.5、输入限制与使用限制

#### 2.5.1输入限制

输入限制指的是用户输入时，输入的字符串的要求，通常有关于长度和内容的要求，这些要求大多数（除磁盘/目录刷新外）未被程序限制，但不这么做可能出现错误。

#### 2.5.2使用限制

指这样使用时合法的，大多数不合法的操作（删除当前目录或包含当前目录的目录除外）均会返回提示且被禁止操作，注意，虽然程序未限制，但 **禁止删除当前目录或包含当前目录的目录** 。

## 三、命令介绍

### 3.0、Gui操作//mark

### 3.1、路径操作

#### 3.1.1查看当前路径

命令：pwd

内容：查看当前目录路径，输出为绝对路径。

输入限制：无

使用限制：无

3.1.2切换目录

命令：cd+空格+$path 示例：cd /user/316

内容：切换至$path的目录。

输入限制：无

使用限制：

1、$path存在，否则会提示&quot;$path No such file or directory&quot;；

2、$path不是文件而是目录，否则会提示&quot;$path is not a directory.&quot;。

### 3.2、目录或文件操作

#### 3.2.1查看目录或文件

命令1：ls+空格+$path 示例：ls /user/316

命令2：ls

内容：查看$path的目录或文件的内容。对于目录，显示该目录下的所有文件和目录（不包括&quot;.&quot;和&quot;..&quot;）。对于文件，显示文件名。&quot;ls&quot;等价于&quot;ls .&quot;。

输入限制：无

使用限制：

1、$path存在，否则会提示&quot;$path No such file or directory&quot;；

2、$path不是文件而是目录，否则会提示&quot;$path is not a directory.&quot;。

#### 3.2.2重命名目录或文件

命令：

内容：

输入限制：?/\*mark\*/ 长度不超过251，只能由大小写字母、数字、小数点&quot;.&quot;和下划线&quot;\_&quot;组成，不得包含空格、中文字符和其他符号。

使用限制：

1、$path存在，否则会提示&quot;$path No such file or directory&quot;；

2、禁止修改根目录名称，$path为根目录时，会提示//mark

### 3.3 目录操作

#### 3.3.1创建目录

命令：mkdir+空格+$path 示例：mkdir /user/316

$path格式： &quot;$father\_path/$dir\_name&quot; 或 &quot;$dir\_name&quot;，后者等价于&quot;./$dir\_name&quot;。其中$father\_path表示被创造目录的上级文件夹的路径，$dir\_name表示目录的名称。

内容：创造$path的目录。

输入限制：$dir\_name长度不超过251，只能由大小写字母、数字、小数点&quot;.&quot;和下划线&quot;\_&quot;组成，不得包含空格、中文字符和其他符号。

使用限制：

1、$father\_path存在，否则会提示&quot;$father\_path No such file or directory&quot;；

2、$path不是文件而是目录，否则会提示&quot;$father\_path is not a directory.&quot;；

3、$dir\_name的长度小于251，否则会提示&quot;The Name should be no longer than 251 chars!&quot;；

4、$path尚未存在，否则会提示&quot;$path is already existed.&quot;；

5、$father\_path未满，否则会提示&quot;The Directory is full!&quot;。

#### 3.3.2删除文件夹

命令1：rmdir+空格+$path 示例：rmdir /user/316

命令2（ **禁止使用** ）：rmdir

内容：删除$path的目录以及目录内的所有内容，命令2等价于&quot;rmdir .&quot;。

输入限制：无

使用限制：

1、$path存在，否则会提示&quot;$path No such file or directory&quot;；

2、$path不是文件而是目录，否则不删除且会提示&quot;$path is not a directory.&quot;；

3、为安全考虑，不允许删除根目录，$path为根目录时，不删除且会提示&quot;rmdir: it is dangerous to operate recursively on &quot;$path&quot;&quot;；

4、出于操作安全， **禁止用户删除当前目录，或包含当前目录的目录** ，但并未从程序上禁止。当前目录被删除后，当前文件夹位置并不会自动切换，认为当前目录为被删除目录被删除前的地址，使用其他命令可能会造成很多混乱或错误。若用户错误操作导致当前目录被删除，请立刻使用&quot;cd /&quot;将当前目录切换回根目录。

### 3.4、文件操作

#### 3.4.1将字符串写入文件

命令：echo+空格+$str+空格+$path 示例：echo helloworld! /user/316/file

$path格式： &quot;father\_path/file\_name&quot; 或 &quot;file\_name&quot;。其中$father\_path表示被创造目录的上级文件夹的路径，$file\_name表示文件的名称。

内容：将$str写入$path文件，若文件存在则覆盖原文件，否则创建文件。

输入限制：$file\_name长度不超过251，只能由大小写字母、数字、小数点&quot;.&quot;和下划线&quot;\_&quot;组成，不得包含空格、中文字符和其他符号。$str长度不超过4096，且不包含空格，不建议加入中文，可能无法正常读取。

使用限制：

1、$path存在时不能为目录，若$path为目录，会提示&quot;$father\_path is not a file.&quot;；

2、$file\_name的长度小于251，否则会提示&quot;The Name should be no longer than 251 chars!&quot;；

3、$father\_path存在，否则会提示&quot;$father\_path No such file or directory&quot;；

4、$father\_path不是文件而是目录，否则会提示&quot;$father\_path is not a directory.&quot;；

5、$father\_path未满，否则会提示&quot;The Directory is full!&quot;；

6、inode（索引）块不能全部被占用，否则会提示&quot;All inodes are used&quot;；

7、block（数据）块不能全部被占用，否则会提示&quot;All blocks are used&quot;。

#### 3.4.2读取文件内容

命令：cat+空格+$path 示例：cat /user/316/file

内容：读取$path文件内容。

输入限制：无。

使用限制：

1、$path存在，否则会提示&quot;$path No such file or directory&quot;；

2、$path不是目录而是文件，否则会提示&quot;$path is not a file.&quot;。

#### 3.4.3删除文件

命令：rm+空格+$path 示例：rm /user/316/file

内容：删除$path文件。

输入限制：无。

使用限制：

1、$path存在，否则会提示&quot;$path No such file or directory&quot;；

2、$path不是目录而是文件，否则会提示&quot;$path is not a file.&quot;。

### 3.5磁盘操作

#### 3.5.1格式化磁盘

命令：format

内容：格式化磁盘，将磁盘中除根目录外的所有内容清空。

使用说明：输入命令后，会提示确认，输入Y确认，输入其他取消格式化；之后会选择模式，输入Q选择快速模式，输入C选择全面模式，输入其他选择取消格式化。

快速模式原理：在不删除根目录的情况下依次删除根目录下的所有内容，相当于没有删除自身信息的rmdir。

全面模式原理：执行创造Filesys.vfs的程序。

#### 3.5.2磁盘/目录刷新

命令：refresh $mode

内容：将程序中记录的内容全部写入文件（$mode为0），或重新读取文件内容（$mode为1）。

输入限制：$mode为0或1，否则会提示&quot;refresh: mode not found, 0 or 1 only&quot;。

使用限制：Filesys.vfs存在，否则会提示&quot; Filesys.vfs is not exist&quot;。

使用说明：程序中不储存block（数据）信息，&quot;refresh 0&quot;只更新inode（索引）和superblock（超级块）信息。正常情况下程序与文件的内容是同步变化的，当文件被替换时会不同步，需要refresh，注意，若用不正常的同名文件替换，很有可能导致程序崩溃。

## 三、小组分工

傅舟涛主要负责磁盘格式化和磁盘刷新的接口、rmdir、echo、cat、rm、磁盘格式化、以及说明文件的大部分内容，瞿凡主要负责初始化、前端（Gui）、大部分接口、pwd、cd、mkdir、ls、重命名、以及说明文件的重命名和Gui部分。fu.cpp、fu.h的内容来自于傅舟涛，其余主要来自于瞿凡。
