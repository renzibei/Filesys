#ifndef FILEVIEW_H
#define FILEVIEW_H

#include <QListWidget>
#include <QLineEdit>
#include <QDialog>
#include "../Filesys.h"

class FileView : public QDialog
{
    Q_OBJECT

public:
    FileView(QWidget *parent = 0);
    ~FileView();
protected slots:
    void slotShowDir(QListWidgetItem * item);
private:
       void showFileInfoList();
private:
       QLineEdit * pLineEditDir;
       QListWidget * pListWidgetFile;
};

#endif // FILEVIEW_H
