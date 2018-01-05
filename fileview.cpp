#include "fileview.h"
#include <iostream>

FileView::FileView(QWidget *parent)
    : QDialog(parent)
{
    this->setWindowIcon(QIcon(":/new/icon/pngsources/vfs_icon.svg"));
    InitDisk();
    setWindowTitle( tr( "Virtual File System" ) );
    WorkPathEdit = new QLineEdit(this);
    WorkPathEdit->setText( tr( "/" ) );
    GoToBtn = new QPushButton;
    GoToBtn->setIcon(QIcon(":/new/icon/pngsources/right.svg"));
    connect(GoToBtn, SIGNAL(clicked()), this, SLOT(slotGoTo()));
    QHBoxLayout * h_layout1 = new QHBoxLayout;
    h_layout1->addWidget(WorkPathEdit);
    h_layout1->addWidget(GoToBtn);
    FileListWidget = new DirList(this);
    FileListWidget->setViewMode(QListView::IconMode);
    UpFolBtn = new QPushButton;
    //UpFolBtn->setFixedWidth(32);
    UpFolBtn->setIcon(QIcon(":/new/icon/pngsources/up.svg"));
    connect(UpFolBtn, SIGNAL(clicked()), this, SLOT(slotGoUpDIr()));
    NewFolBtn = new QPushButton;
    NewFolBtn->setText(tr("新建文件夹"));
    //NewFolBtn->setFixedHeight(40);
    connect(NewFolBtn, SIGNAL(clicked()), FileListWidget, SLOT(slotNewDir()));
    QHBoxLayout * h_layout2 = new QHBoxLayout;
    h_layout2->addWidget(NewFolBtn);
    h_layout2->addWidget(UpFolBtn);
    QVBoxLayout * v_layout = new QVBoxLayout(this);
    v_layout->addLayout(h_layout1);
    v_layout->addLayout(h_layout2);
    v_layout->addWidget( FileListWidget );
    this->setLayout(v_layout);
    connect( FileListWidget, SIGNAL( itemDoubleClicked( QListWidgetItem * ) ), this, SLOT( slotShowDir( QListWidgetItem * ) ) );
    UpdateFileList();

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
    QAction *edit_file_action = new QAction("编辑", this);
    if(this->itemAt(mapFromGlobal(QCursor::pos())) != NULL)
    {
        popMenu->addAction(delete_action);
        toDeleteItem = this->itemAt(mapFromGlobal(QCursor::pos()));
        QByteArray file_path = toDeleteItem->text().toLatin1();
        int file_inode_id = GetPathInode(file_path.data());
        if(inodes[file_inode_id].i_mode == 1)
            popMenu->addAction(edit_file_action);
    }
    else {
        popMenu->addAction(add_folder_action);
        popMenu->addAction(add_file_action);
    }
    connect(add_folder_action, SIGNAL(triggered()), this, SLOT(slotNewDir()));
    connect(delete_action, SIGNAL(triggered()), this, SLOT(slotDelete()));
    connect(edit_file_action, SIGNAL(triggered(bool)), this, SLOT(slotEditFile()));
    popMenu->exec(QCursor::pos());
}

void DirList::slotEditFile()
{
    QByteArray file_path = toDeleteItem->text().toLatin1();
    EditTextWidget *text_edit_dialog = new EditTextWidget(this, file_path.data());
    text_edit_dialog->show();
}

void DirList::slotDelete()
{
    //QMessageBox::information(vfs_gui, "", "正常");
    QByteArray filePathStr = toDeleteItem->text().toLatin1();
    char *filePath = filePathStr.data();
    /*
    QString debugtext(filePath);
    QMessageBox::information(vfs_gui, "", debugtext);

    debugtext.setNum(d_inodeid);
    QMessageBox::information(vfs_gui, "", debugtext);
    */
    int d_inodeid = GetPathInode(filePath);
    if(d_inodeid < 0) {
        QMessageBox::critical(this, "", "文件不存在");
    }
    int status_value = -2;
    if(inodes[d_inodeid].i_mode == 0)
        status_value = delete_directory(d_inodeid);
    else
        status_value = delete_file(d_inodeid);
    if(status_value < 0)
        QMessageBox::critical(this, "", "删除失败");
    vfs_gui->UpdateFileList();

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
}


void DirList::slotCreateFolder()
{
    QByteArray dirname = NameEdit->text().toLatin1();
    char* dirpath = dirname.data();
    int createstatus = MakeFolder(dirpath);
    if(createstatus == -1)
        QMessageBox::critical(this, "", tr("文件不存在"));
    else if(createstatus == -2)
        QMessageBox::warning(this, "", tr("文件已存在"));
    else if(createstatus == -3)
        QMessageBox::critical(this, "", tr("文件夹已满"));
    NewNameD->close();
    vfs_gui->UpdateFileList();
}

void FileView::slotGoTo()
{
    QByteArray pathStr = WorkPathEdit->text().toLatin1();
    char *path = pathStr.data();
    int GoToStatus = GetPathInode(path, 1);
    if(GoToStatus == -1)
        QMessageBox::critical(this, "", tr("路径错误"));
    else if(inodes[GoToStatus].i_mode == 1)
        QMessageBox::critical(this, "", tr("目标是文件"));
    UpdateFileList();
}

void FileView::UpdateFileList()
{
    char abopath[input_buffer_length] = {0};
    GetAboPath(abopath);
    QString Qabopath(abopath);
    WorkPathEdit->setText( Qabopath );
    FileListWidget->clear();
    FILE *vfs = fopen(filename, "rb");
    char dir_name[253] = {0};
    int dir_entry_id = -1;
    for (int i = 0; i < 16; ++i)
    {
        fseek(vfs, DataBlkPos(inodes[wkpath->dir_inode].i_blocks[0]), SEEK_SET);
        fseek(vfs, DirsPos(i), SEEK_CUR);
        memset(dir_name, 0, sizeof(dir_name));
        fread(dir_name, sizeof(char), 252, vfs);
        fread(&dir_entry_id, sizeof(int), 1, vfs);
        if(strlen(dir_name) != 0) {
            QString fileName(dir_name);
            if (inodes[dir_entry_id].i_mode == 0) {
                QListWidgetItem * pTmp = new QListWidgetItem( QIcon( ":/new/icon/pngsources/folder2.svg" ), fileName );
                FileListWidget->addItem( pTmp );
            }
            else {
                QListWidgetItem * pTmp = new QListWidgetItem( QIcon( ":/new/icon/pngsources/file1.svg" ), fileName );
                FileListWidget->addItem( pTmp );
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
       UpdateFileList();
}

void FileView::slotGoUpDIr()
{
    GetPathInode("..",1);
    UpdateFileList();
}
