#include "textedit.h"

EditTextWidget::EditTextWidget(QWidget *parent, char _file_name[])
    :QDialog(parent)
{
    IsSaved = 1;
    this->setWindowIcon(QIcon(":/new/icon/file1.svg"));
    QuitBtn = new QPushButton;
    SaveBtn = new QPushButton;
    UndoBtn = new QPushButton;
    this->setWindowTitle(_file_name);
    QuitBtn->setText(tr("退出"));
    SaveBtn->setText(tr("保存"));
    UndoBtn->setText(tr("撤销"));
    memset(content_char, 0, sizeof(content_char));
    memset(file_name_str, 0 ,sizeof(file_name_str));
    strncpy(file_name_str, _file_name, 252);
    DoCat(file_name_str, content_char);
    theTextEdit = new QTextEdit(content_char);
    QHBoxLayout *textHLayout = new QHBoxLayout;
    textHLayout->addWidget(QuitBtn);
    textHLayout->addWidget(SaveBtn);
    textHLayout->addWidget(UndoBtn);
    QVBoxLayout *textVLayout = new QVBoxLayout;
    textVLayout->addLayout(textHLayout);
    textVLayout->addWidget(theTextEdit);
    this->setLayout(textVLayout);
    connect(QuitBtn, SIGNAL(clicked()), this, SLOT(slotQuit()));
    connect(SaveBtn, SIGNAL(clicked()), this, SLOT(slotSave()));
    connect(UndoBtn, SIGNAL(clicked()), theTextEdit, SLOT(undo()));
    connect(theTextEdit, SIGNAL(textChanged()), this, SLOT(slotChangeSaveStatus()));
}

EditTextWidget::~EditTextWidget()
{

}

void EditTextWidget::slotChangeSaveStatus()
{
    IsSaved = 0;
}

void EditTextWidget::slotQuit()
{
    if(!IsSaved) {
        switch(QMessageBox::question(this,
                                     "",
                                     tr("更改还未保存，是否保存后退出？"),
                                     QMessageBox::Discard | QMessageBox::Cancel | QMessageBox::Save,
                                     QMessageBox::Cancel ))
        {
        case QMessageBox::Cancel:
            break;
        case QMessageBox::Discard:
            this->close();
            break;
        case QMessageBox::Save:
            if(slotSave() == 0)
                this->close();
            break;
        default:
            break;
        }
    }
    else this->close();
}

int EditTextWidget::slotSave()
{
    QByteArray temp_text_str = theTextEdit->toPlainText().toLatin1();
    int return_value = 0;
    if(temp_text_str.size() >= 4096) {
        QMessageBox::warning(this, "", tr("文件内容不能超过4096字符"));
        return_value = -1;
    }
    else {
        char *content_text_p = temp_text_str.data();
        echo(file_name_str, content_text_p);
        IsSaved = 1;
        return_value = 0;
    }
    return return_value;
}
