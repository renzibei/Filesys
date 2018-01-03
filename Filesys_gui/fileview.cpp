#include "fileview.h"
#include <QLayout>
#include <iostream>

FileView::FileView(QWidget *parent)
    : QDialog(parent)
{
    InitDisk();
    setWindowTitle( tr( "Virtual File System" ) );
    pLineEditDir = new QLineEdit(this);
    pLineEditDir->setText( tr( "/" ) );
    GoToBtn = new QPushButton;
    GoToBtn->setIcon(QIcon(":/new/icon/pngsources/right.svg"));
    connect(GoToBtn, SIGNAL(clicked()), this, SLOT(slotGoTo()));
    QHBoxLayout * pHLayout1 = new QHBoxLayout;
    pHLayout1->addWidget(pLineEditDir);
    pHLayout1->addWidget(GoToBtn);
    pListWidgetFile = new DirList(this);
    pListWidgetFile->setViewMode(QListView::IconMode);
    UpFolBtn = new QPushButton;
    //UpFolBtn->setFixedWidth(32);
    UpFolBtn->setIcon(QIcon(":/new/icon/pngsources/up.svg"));
    connect(UpFolBtn, SIGNAL(clicked()), this, SLOT(slotGoUpDIr()));
    NewFolBtn = new QPushButton;
    NewFolBtn->setText(tr("新建文件夹"));
    //NewFolBtn->setFixedHeight(40);
    connect(NewFolBtn, SIGNAL(clicked()), pListWidgetFile, SLOT(slotNewDir()));
    QHBoxLayout * pHLayout2 = new QHBoxLayout;
    pHLayout2->addWidget(NewFolBtn);
    pHLayout2->addWidget(UpFolBtn);
    QVBoxLayout * pVLayout = new QVBoxLayout(this);
    pVLayout->addLayout(pHLayout1);
    pVLayout->addLayout(pHLayout2);
    pVLayout->addWidget( pListWidgetFile );
    this->setLayout(pVLayout);
    connect( pListWidgetFile, SIGNAL( itemDoubleClicked( QListWidgetItem * ) ), this, SLOT( slotShowDir( QListWidgetItem * ) ) );
    showFileInfoList();

}




DirList::DirList(QWidget *parent)
    : QListWidget(parent)
{
    setContextMenuPolicy(Qt::DefaultContextMenu);
}

DirList::~DirList()
{

}


void DirList::contextMenuEvent( QContextMenuEvent * event )
{
    QMenu* popMenu = new QMenu(this);
    QAction *add_folder_action = new QAction("新建文件夹", this);
    QAction *add_file_action = new QAction("新建文件", this);
    QAction *delete_action = new QAction("删除",this);
    popMenu->addAction(add_folder_action);
    popMenu->addAction(add_file_action);
    if(this->itemAt(mapFromGlobal(QCursor::pos())) != NULL)
    {
        popMenu->addAction(delete_action);
    }
    connect(add_folder_action, SIGNAL(triggered()), this, SLOT(slotNewDir()));
    popMenu->exec(QCursor::pos()); // 菜单出现的位置为当前鼠标的位置
}

void DirList::SentDirName()
{

}

void DirList::slotNewDir()
{
    NewNameD = new QDialog();
    NameEdit = new QLineEdit();
    QLabel *nameLabel = new QLabel();
    QPushButton *finishBtn = new QPushButton();
    finishBtn->setText(tr("完成"));
    nameLabel->setText(tr("请输入文件名"));
    QVBoxLayout *nameLayout = new QVBoxLayout;
    nameLayout->addWidget(nameLabel);
    nameLayout->addWidget(NameEdit);
    nameLayout->addWidget(finishBtn);
    NewNameD->setLayout(nameLayout);
    NewNameD->show();
    connect(finishBtn, SIGNAL(clicked()), this, SLOT(slotCreateFolder()));

    //connect(finishBtn, SIGNAL(clicked()), this, )
}
/*
StandardButton QMessageBox::information
(
    QWidget* parent,
    const QString &title,
    const QString& text,
    StandardButtons button=Ok,
    StandardButton defaultButton=NoButton

);*/


void DirList::slotCreateFolder()
{
    QByteArray dirname = NameEdit->text().toLatin1();
    char* dirpath = dirname.data();
    int createstatus = MakeFolder(dirpath);
    if(createstatus == -1)
        QMessageBox::critical(this, "", tr("文件不存在"));
    else if(createstatus == -2)
        QMessageBox::critical(this, "", tr("文件已存在"));
    else if(createstatus == -3)
        QMessageBox::critical(this, "", tr("文件夹已满"));
    NewNameD->close();
    vfs_gui->showFileInfoList();
}

void FileView::slotGoTo()
{
    QByteArray pathStr = pLineEditDir->text().toLatin1();
    char *path = pathStr.data();
    int GoToStatus = GetPathInode(path, 1);
    if(GoToStatus == -1)
        QMessageBox::critical(this, "", tr("路径错误"));
    else if(inodes[GoToStatus].i_mode == 1)
        QMessageBox::critical(this, "", tr("目标是文件"));
    showFileInfoList();
}

void FileView::showFileInfoList()
{
    char abopath[input_buffer_length] = {0};
    GetAboPath(abopath);
    QString Qabopath(abopath);
    pLineEditDir->setText( Qabopath );
    pListWidgetFile->clear();
    FILE *vfs = fopen(filename, "rb");
    char dir_name[253] = {0};
    int dir_entry_id = -1;
    for (int i = 0; i < 16; ++i)
    {
           //QFileInfo tmpFileInfo = list.at( i );
        fseek(vfs, DataBlkPos(inodes[wkpath->dir_inode].i_blocks[0]), SEEK_SET);
        fseek(vfs, DirsPos(i), SEEK_CUR);
        memset(dir_name, 0, sizeof(dir_name));
        fread(dir_name, sizeof(char), 252, vfs);
        fread(&dir_entry_id, sizeof(int), 1, vfs);
        if(strlen(dir_name) != 0) {
            QString fileName(dir_name);
            if (inodes[dir_entry_id].i_mode == 0) {
                QListWidgetItem * pTmp = new QListWidgetItem( QIcon( ":/new/icon/pngsources/folder2.svg" ), fileName );
                pListWidgetFile->addItem( pTmp );
            }
            else {
                QListWidgetItem * pTmp = new QListWidgetItem( QIcon( ":/new/icon/pngsources/file1.svg" ), fileName );
                pListWidgetFile->addItem( pTmp );
            }
        }
    }
    fclose(vfs);
}

FileView::~FileView()
{

}

void FileView::slotShowDir( QListWidgetItem * item )
{
       QString str = item->text();
       QByteArray latinstr = str.toLatin1();
       char *subDir = latinstr.data();

       ChangeDir(subDir);

       //dir.setPath( pLineEditDir->text() );
       //dir.cd( str );



       showFileInfoList();
}

void FileView::slotGoUpDIr()
{
    GetPathInode("..",1);
    showFileInfoList();
}
