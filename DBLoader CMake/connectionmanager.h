#ifndef CONNECTIONMANAGER_H
#define CONNECTIONMANAGER_H

#include <QMainWindow>

namespace Ui {
class ConnectionManager;
}

class ConnectionManager : public QMainWindow
{
    Q_OBJECT

public:
    explicit ConnectionManager(QWidget *parent = nullptr);
    ~ConnectionManager();
private:
    QString last_conname = "";

private slots:
    void updateList();

    void on_connection_ListWidget_currentTextChanged(const QString &currentText);

    void on_save_pushButton_clicked();

    void on_cancel_pushButton_clicked();

    void on_new_pushButton_clicked();

    void on_Delete_pushButton_clicked();

    void on_showPassword_checkBox_checkStateChanged(const Qt::CheckState &arg1);

private:
    Ui::ConnectionManager *ui;
};

#endif // CONNECTIONMANAGER_H
