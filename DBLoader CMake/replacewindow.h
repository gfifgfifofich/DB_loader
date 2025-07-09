#ifndef REPLACEWINDOW_H
#define REPLACEWINDOW_H

#include <QMainWindow>

namespace Ui {
class ReplaceWindow;
}

class ReplaceWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ReplaceWindow(QWidget *parent = nullptr);
    ~ReplaceWindow();


    int start_line = 0;
    int end_line = 1000000;
    QString replaceWhat = "zxcasdnothing_realy_like_nothing_at_allzxcasd";
    QString replaceWith = "";


    void Init();

private slots:
    void on_lineEdit_textChanged(const QString &arg1);

    void on_lineEdit_2_textChanged(const QString &arg1);

    void on_spinBox_valueChanged(int arg1);

    void on_spinBox_2_valueChanged(int arg1);

    void on_checkBox_checkStateChanged(const Qt::CheckState &arg1);


    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

private:
    Ui::ReplaceWindow *ui;
};

#endif // REPLACEWINDOW_H
