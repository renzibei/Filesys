#include "fileview.h"
#include <QApplication>
#define _CRT_SECURE_NO_WARNINGS

_super_block sbks;
_inode inodes[4096];
FileView *vfs_gui;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    vfs_gui = new FileView;
    vfs_gui->show();

    return a.exec();
}
