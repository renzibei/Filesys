#ifndef THETEXTEDIT_H
#define THETEXTEDIT_H
#include "fileview.h"

class EditTextWidget : public QDialog
{
    Q_OBJECT
public:
    explicit EditTextWidget(QWidget *parent = 0, char _file_name[] = 0);
    ~EditTextWidget();

private:
    QTextEdit *theTextEdit;
    QPushButton *QuitBtn;
    QPushButton *SaveBtn;
    QPushButton *UndoBtn;
    char content_char[4097];
    char file_name_str[252];
    bool IsSaved;
protected slots:
    void slotQuit();
    int slotSave();
    void slotChangeSaveStatus();
};
#endif // TEXTEDIT_H
