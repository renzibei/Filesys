#include "fileview.h"
#include <QApplication>
#define _CRT_SECURE_NO_WARNINGS

_super_block sbks;
_inode inodes[4096];

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    FileView w;
    w.show();

    return a.exec();
}
