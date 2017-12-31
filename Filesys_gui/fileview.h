#ifndef FILEVIEW_H
#define FILEVIEW_H

#include <QListWidget>
#include <QLineEdit>
#include <QDialog>
#include <QPushButton>
#include <QMenu>
#include <QContextMenuEvent>
#include <QLabel>
#include <QMessageBox>
#include "../Filesys.h"



class DirList :public QListWidget
{
    Q_OBJECT
public:
    DirList(QWidget *parent = 0);
    ~DirList();
protected slots:
    void slotNewDir();
    void SentDirName();
    void slotCreateFolder();
protected:
    void contextMenuEvent(QContextMenuEvent * event);

private:
    QLineEdit *NameEdit;
    QDialog *NewNameD;
};

class FileView : public QDialog
{
    Q_OBJECT

public:
    FileView(QWidget *parent = 0);
    void showFileInfoList();
    ~FileView();
protected slots:
    void slotShowDir(QListWidgetItem * item);


private:


       QLineEdit * pLineEditDir;
       DirList * pListWidgetFile;
       QPushButton * NewFolBtn;
};

extern FileView* vfs_gui;

#endif // FILEVIEW_H
