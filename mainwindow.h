#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <vector>
#include "codeeditor.h"
#include "table.h"
#include <QSqlDatabase>
#include <QSqlError>

#include <QTableView>

#include <QSqlQuery>
#include <QSqlResult>
#include <QSqlRecord>
QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    std::vector<Table*> tableWindows;
    QString lastsql = "";
    CodeEditor* cd = nullptr;
private slots:

    void on_pushButton_clicked();

    void on_pushButton_5_clicked();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
