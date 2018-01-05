#include "fileview.h"
#include <iostream>

FileView::FileView(QWidget *parent)
    : QDialog(parent)
{
    this->setWindowIcon(QIcon(":/new/icon/vfs_icon.svg"));
    InitDisk();
    setWindowTitle( tr( "Virtual File System" ) );
    WorkPathEdit = new QLineEdit(this);
    WorkPathEdit->setText( tr( "/" ) );
    GoToBtn = new QPushButton;
    GoToBtn->setIcon(QIcon(":/new/icon/right.svg"));
    connect(GoToBtn, SIGNAL(clicked()), this, SLOT(slotGoTo()));
    QHBoxLayout * h_layout1 = new QHBoxLayout;
    h_layout1->addWidget(WorkPathEdit);
    h_layout1->addWidget(GoToBtn);
    FileListWidget = new DirList(this);
    FileListWidget->setViewMode(QListView::IconMode);
    UpFolBtn = new QPushButton;
    //UpFolBtn->setFixedWidth(32);
    UpFolBtn->setIcon(QIcon(":/new/icon/up.svg"));
    connect(UpFolBtn, SIGNAL(clicked()), this, SLOT(slotGoUpDIr()));
    NewFolBtn = new QPushButton;
    NewFolBtn->setIcon(QIcon(":/new/icon/new_folder.svg"));
    NewFileBtn = new QPushButton;
    NewFileBtn->setIcon(QIcon(":/new/icon/new_doc.svg"));
    //NewFolBtn->setText(tr("新建文件夹"));
    //NewFolBtn->setFixedHeight(40);
    connect(NewFolBtn, SIGNAL(clicked()), FileListWidget, SLOT(slotNewDir()));
    connect(NewFileBtn, SIGNAL(clicked()), FileListWidget, SLOT(slotNewFile()));
    QHBoxLayout * h_layout2 = new QHBoxLayout;
    h_layout2->addWidget(NewFolBtn);
    h_layout2->addWidget(NewFileBtn);
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
    QAction *rename_action = new QAction("重命名", this);
    QAction *delete_action = new QAction("删除",this);
    QAction *edit_file_action = new QAction("编辑", this);
    if(this->itemAt(mapFromGlobal(QCursor::pos())) != NULL)
    {
        popMenu->addAction(delete_action);
        popMenu->addAction(rename_action);
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
    connect(add_file_action, SIGNAL(triggered()), this, SLOT(slotNewFile()));
    connect(delete_action, SIGNAL(triggered()), this, SLOT(slotDelete()));
    connect(edit_file_action, SIGNAL(triggered()), this, SLOT(slotEditFile()));
    connect(rename_action, SIGNAL(triggered()), this, SLOT(slotReName()));
    popMenu->exec(QCursor::pos());
}

void DirList::slotReName()
{
    QString oldFileName = toDeleteItem->text(), newFileName;
    int JudgeStatus = GetNewName(this, newFileName, 0, oldFileName);
    if(JudgeStatus == 0)
        ReName(oldFileName.toLatin1().data(), newFileName.toLatin1().data());
    vfs_gui->UpdateFileList();
}

int GetNewName(QWidget *parent, QString &the_new_file_name, int judge_type, const QString &the_old_name)
{
    QString new_file_name, to_display;
    bool breakFlag = 0, changeFlag = 0;
    int ReturnStatus = 0;
    if(judge_type == 0)
        to_display = QWidget::tr("请输入文件名");
    else to_display = QWidget::tr("请输入文件夹名");
    if(the_old_name != NULL)
        new_file_name = the_old_name;
    for(; !breakFlag; ) {
        new_file_name = QInputDialog::getText(parent, "", to_display, QLineEdit::Normal, new_file_name, &changeFlag);
        if(changeFlag && new_file_name.size() > 0) {
            if(new_file_name.size() < 252) {
                int tar_inode_id = GetPathInode(new_file_name.toLatin1().data());
                if(tar_inode_id < 0 || (the_old_name != NULL && tar_inode_id == GetPathInode(the_old_name.toLatin1().data()))) {
                    the_new_file_name = new_file_name;
                    //echo(new_file_name.toLatin1().data(), " ");
                    breakFlag = 1;
                    ReturnStatus = 0;
                }
                else {
                    QMessageBox::warning(parent, "", QWidget::tr("文件名与当前文件重复"));
                    ReturnStatus = -4;
                }
            }
            else {
                QMessageBox::warning(parent, "", QWidget::tr("文件名长度不能超过251字符"));
                ReturnStatus = -1;
            }

        }
        else if(changeFlag) {
            QMessageBox::warning(parent, "", QWidget::tr("文件名不能为空"));
            ReturnStatus = -2;
        }
        else {
            breakFlag = 1;
            ReturnStatus = -3;
        }
    }
    return ReturnStatus;
}


void DirList::slotNewFile()
{
    if(find_free_dir_entry(wkpath->dir_inode) == -1) {
        QMessageBox::critical(this, "", tr("当前文件夹已满"));
    }
    else {
        QString new_file_name;
        int JudgeFlag = GetNewName(this, new_file_name);
        if(JudgeFlag == 0)
            DoEcho(new_file_name.toLatin1().data(), " ");
        vfs_gui->UpdateFileList();
    }
}


void DirList::slotEditFile(QString *the_file_name)
{
    QByteArray file_path;
    if(the_file_name == NULL)
        file_path = toDeleteItem->text().toLatin1();
    else file_path = the_file_name->toLatin1();
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
    if(find_free_dir_entry(wkpath->dir_inode) == -1) {
        QMessageBox::critical(this, "", tr("当前文件夹已满"));
    }
    else {
        QString new_folder_name;
        int JudgeFlag = GetNewName(this, new_folder_name, 1);
        if(JudgeFlag == 0)
            MakeFolder(new_folder_name.toLatin1().data());
        vfs_gui->UpdateFileList();
    }
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
    //cout << this << endl;
    QString Qabopath(abopath);
    WorkPathEdit->setText( Qabopath );
    FileListWidget->clear();
    FILE *vfs = fopen(filename, "rb");
    char dir_name[253] = {0};
    int dir_entry_id = -1;
    for (int i = 2; i < 16; ++i)
    {
        fseek(vfs, DataBlkPos(inodes[wkpath->dir_inode].i_blocks[0]), SEEK_SET);
        fseek(vfs, DirsPos(i), SEEK_CUR);
        memset(dir_name, 0, sizeof(dir_name));
        fread(dir_name, sizeof(char), 252, vfs);
        fread(&dir_entry_id, sizeof(int), 1, vfs);
        if(strlen(dir_name) != 0) {
            QString fileName(dir_name);
            if (inodes[dir_entry_id].i_mode == 0) {
                QListWidgetItem * pTmp = new QListWidgetItem( QIcon( ":/new/icon/folder2.svg" ), fileName );
                FileListWidget->addItem( pTmp );
            }
            else {
                QListWidgetItem * pTmp = new QListWidgetItem( QIcon( ":/new/icon/file1.svg" ), fileName );
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
       int Tar_Inode_id = GetPathInode(subDir);
       if(inodes[Tar_Inode_id].i_mode == 0)
            ChangeDir(subDir);
       else {
            //FileListWidget->toDeleteItem = item;
            FileListWidget->slotEditFile(&str);
       }
       UpdateFileList();
}

void FileView::slotGoUpDIr()
{
    GetPathInode("..",1);
    UpdateFileList();
}
