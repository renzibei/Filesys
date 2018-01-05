#include "fileview.h"
#include <QApplication>
#define _CRT_SECURE_NO_WARNINGS

_super_block sbks;
_inode inodes[4096];
FileView *vfs_gui;
QTranslator *tran;
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    tran = new QTranslator;
    bool ok = 0;
    ok = tran->load("qt_zh_CN.qm",":/trans/");
    a.installTranslator(tran);
    vfs_gui = new FileView;
    vfs_gui->show();

    return a.exec();
}
