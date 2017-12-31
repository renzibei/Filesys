#ifndef FILEVIEW_H
#define FILEVIEW_H

#include <QDialog>

class FileView : public QDialog
{
    Q_OBJECT

public:
    FileView(QWidget *parent = 0);
    ~FileView();
};

#endif // FILEVIEW_H
