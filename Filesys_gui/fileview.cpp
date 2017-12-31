#include "fileview.h"
#include <QLayout>

FileView::FileView(QWidget *parent)
    : QDialog(parent)
{
    InitDisk();
    setWindowTitle( tr( "Virtual File System" ) );
    pLineEditDir = new QLineEdit(this);
    pLineEditDir->setText( tr( "/" ) );
    pListWidgetFile = new QListWidget(this);
    QVBoxLayout * pVLayout = new QVBoxLayout( this );
    pVLayout->addWidget( pLineEditDir );
    pVLayout->addWidget( pListWidgetFile );
    connect( pListWidgetFile, SIGNAL( itemDoubleClicked( QListWidgetItem * ) ), this, SLOT( slotShowDir( QListWidgetItem * ) ) );
    QStringList sList;
    sList << "*";
    showFileInfoList();

}

FileView::~FileView()
{

}

void FileView::showFileInfoList()
{
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
               if (inodes[dir_entry_id].i_mode == 0)
              {
                     QListWidgetItem * pTmp = new QListWidgetItem( QIcon( "../pngsources/folder.png" ), fileName );
                     pListWidgetFile->addItem( pTmp );
              }
              else
              {
                     QListWidgetItem * pTmp = new QListWidgetItem( QIcon( "../pngsources/filenew 4.png" ), fileName );
                     pListWidgetFile->addItem( pTmp );
              }
           }
       }
       fclose(vfs);
}

void FileView::slotShowDir( QListWidgetItem * item )
{
       QString str = item->text();
       QByteArray latinstr = str.toLatin1();
       char *subDir = latinstr.data();

       ChangeDir(subDir);

       //dir.setPath( pLineEditDir->text() );
       //dir.cd( str );
       char abopath[input_buffer_length] = {0};

       GetAboPath(abopath);
       QString Qabopath(abopath);
       pLineEditDir->setText( Qabopath );
       showFileInfoList();
}
