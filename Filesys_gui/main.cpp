#include "fiileview.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    FiileView w;
    w.show();

    return a.exec();
}
