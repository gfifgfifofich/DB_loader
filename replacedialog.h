#ifndef REPLACEDIALOG_H
#define REPLACEDIALOG_H

#include <QDialog>
#include "codeeditor.h"
namespace Ui {
class replaceDialog;
}

class replaceDialog : public QDialog
{
    Q_OBJECT

public:
    explicit replaceDialog(QWidget *parent = nullptr);
    ~replaceDialog();
    CodeEditor* trg_editor = nullptr;

    int start_line = 0;
    int end_line = 1000000;
    QString replaceWhat = "zxcasdnothing_realy_like_nothing_at_allzxcasd";
    QString replaceWith = "";


    void Init();

private slots:
    void on_spinBox_2_valueChanged(int arg1);

    void on_spinBox_valueChanged(int arg1);

    void on_lineEdit_textChanged(const QString &arg1);

    void on_lineEdit_2_textChanged(const QString &arg1);

    void on_checkBox_checkStateChanged(const Qt::CheckState &arg1);

private:
    Ui::replaceDialog *ui;
};

#endif // REPLACEDIALOG_H
