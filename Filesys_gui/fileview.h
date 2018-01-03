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
    void slotDelete();
protected:
    void contextMenuEvent(QContextMenuEvent * event);

private:
    QLineEdit *NameEdit;
    QDialog *NewNameD;
    QListWidgetItem * toDeleteItem;
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
    void slotGoUpDIr();
    void slotGoTo();
private:


       QLineEdit * WorkPathEdit;
       DirList * FileListWidget;
       QPushButton * NewFolBtn;
       QPushButton * UpFolBtn;
       QPushButton * GoToBtn;
};

extern FileView* vfs_gui;

#endif // FILEVIEW_H