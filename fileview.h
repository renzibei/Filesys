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
#include <QTextEdit>
#include <QInputDialog>
#include <QLayout>
#include <QTranslator>

#include "textedit.h"
#include "Filesys.h"

int GetNewName(QWidget *parent, QString &the_new_file_name, int judge_type = 0, const QString &the_old_name = NULL);

class DirList :public QListWidget
{
    Q_OBJECT
public:
    DirList(QWidget *parent = 0);
    ~DirList();
public slots:
    void slotEditFile(QString *the_file_name = NULL);
protected slots:
    void slotNewDir();
    void slotNewFile();
    void SentDirName();
   // void slotCreateFolder();
    void slotDelete();
    void slotReName();


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
    void UpdateFileList();
    ~FileView();
protected slots:
    void slotShowDir(QListWidgetItem * item);
    void slotGoUpDIr();
    void slotGoTo();

private:


       QLineEdit * WorkPathEdit;
       DirList * FileListWidget;
       QPushButton * NewFolBtn;
       QPushButton * NewFileBtn;
       QPushButton * UpFolBtn;
       QPushButton * GoToBtn;
};



extern FileView* vfs_gui;

#endif // FILEVIEW_H
