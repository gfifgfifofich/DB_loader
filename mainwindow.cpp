#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QThread>
#include <qdatetime.h>

inline QSqlDatabase db;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->comboBox->addItem("KTKDB_DB2");
    ui->comboBox->addItem("WMDB_STB_REPORTS");
    ui->comboBox->addItem("KTK_DWH");
    ui->comboBox->addItem("WM_DWH");

    cd = new CodeEditor();
    ui->horizontalLayout_2->addWidget(cd);

    cd->toPlainText();

}



MainWindow::~MainWindow()
{
    db.close();
    for(int i=0;i<tableWindows.size();i++)
        tableWindows[i]->close();
    qDebug()<<"closed";
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    db.close();
    db = QSqlDatabase::addDatabase("QODBC");
    db.setDatabaseName(ui->comboBox->currentText());
    db.setUserName(ui->lineEdit_2->text());
    db.setPassword(ui->lineEdit_1->text());
    bool ok = db.open();
    if(ok)
        qDebug() << "db opened";
    else
        qDebug() << "nope: "<< db.lastError().text();
}


void Func(Table* tbl, QString conname,QString dbname,QString usrname,QString password)
{
    tbl->runSqlAsync(conname, dbname, usrname, password,true);
    return;
}
inline int thrnum =0;
#include <fstream>
void MainWindow::on_pushButton_5_clicked()
{
    QString str = "sqlBackup/";
    str += QTime::currentTime().toString();
    str +=".sql";
    str.replace(":"," ");
    qDebug()<<str;
    std::ofstream stream (str.toStdString());
    stream << cd->toPlainText().toStdString();
    stream.close();

    str = cd->toPlainText();


    tableWindows.push_back(new Table());
    tableWindows.back()->sqlCode = str;
    tableWindows.back()->cd->setPlainText(str);
    tableWindows.back()->show();
    thrnum++;
    tableWindows.back()->conName = QVariant(thrnum).toString();
    tableWindows.back()->runSqlAsync(QVariant(thrnum).toString(), ui->comboBox->currentText(), ui->lineEdit_2->text(),  ui->lineEdit_1->text(),true);

}

