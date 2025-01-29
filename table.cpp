#include "table.h"
#include "datastorage.h"
#include "ui_table.h"

#include <QSqlDatabase>
#include <QSqlError>

#include <QTableView>

#include <QSqlQuery>
#include <QSqlResult>
#include <QSqlRecord>
#include "QXlsx-master/QXlsx/header/xlsxdocument.h"
#include "QXlsx-master/QXlsx/header/xlsxworkbook.h"
#include <QThread>
#include <qdir.h>
#include <qmutex.h>
#include <QShortcut>
#include <fstream>
#include <QDesktopServices>

inline DataStorage userDS;

Table::Table(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Table)
{
    ui->setupUi(this);



    cd = new CodeEditor();
    ui->horizontalLayout_3->addWidget(cd);
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_E), this, SLOT(on_pushButton_3_clicked()));
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Enter), this, SLOT(on_pushButton_3_clicked()));

    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_S), this, SLOT(on_pushButton_2_clicked()));
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_O), this, SLOT(OpenFile()));
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Space), this, SLOT(FillSuggest()));
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_N), this, SLOT(OpenNewWindow()));

    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_I), this, SLOT(ShowIterateWindow()));



    iw.Init();
    ui->horizontalLayout_3->addLayout(&iw.iter_layout);
    connect( &iw.button, SIGNAL(pressed()), this, SLOT(IterButtonPressed()));

    connect( cd, SIGNAL(s_suggestedName()), this, SLOT(updatesuggestion()), Qt::QueuedConnection );
    connect( this, SIGNAL(execdReady()), this, SLOT(ThreadReady()), Qt::QueuedConnection );

    userDS.Load("userdata.txt");

    QStringList strl;
    for( auto x : userDS.data["UserDBs"])
    {
        qDebug() << x.first << "    ";
        //ui->comboBox->addItem(x.first.c_str());
        strl.push_back(x.first.c_str());
    }
    ui->comboBox->addItems(strl);
    ui->comboBox->setCurrentText(userDS.GetProperty("User","lastDBName").c_str());
    ui->lineEdit_2->setText(userDS.GetProperty("User","name").c_str());
    ui->lineEdit_3->setText(userDS.GetProperty("User","password").c_str());
    userDS.Save("userdata.txt");



    connectDB(conName , ui->comboBox->currentText(),ui->lineEdit_2->text(),ui->lineEdit_3->text());


    QFile file("stock.sql");
    if (file.open(QFile::ReadOnly | QFile::Text))
        cd->setPlainText(file.readAll());

    cd->setPlainText(cd->toPlainText());


}



inline int thrnum;

QTime lastMultiRunPress = QTime::currentTime();

void Table::IterButtonPressed()
{

    int secs = QTime::currentTime().minute() * 60 + QTime::currentTime().second();
    int secswas = lastMultiRunPress.minute() * 60 + lastMultiRunPress.second();

    if(secs - secswas < 5)
        return;
    lastMultiRunPress = QTime::currentTime();
    int start = iw.sb1.value();
    int end = iw.sb2.value();
    int step = iw.sb3.value();
    for(int it = start; it <=end;it+=step)
    {
        QString formatedSql = "";
        QString text =cd->toPlainText();
        QString token = iw.nameline.text();
        QStringList strl = text.split(token);
        if(strl.size()>1)
        {
            formatedSql.append(strl[0]);
            for(int i = 1;i<strl.size();i++)
            {
                formatedSql.append(QVariant(it).toString());
                formatedSql.append(strl[i]);
            }
        }
        qDebug() << formatedSql;

        QString str = cd->toPlainText();
        Table* tbl = new Table();
        tbl->sqlCode = formatedSql;
        tbl->cd->setPlainText(formatedSql);



        tbl->autofilename = ui->lineEdit->text();
        if(tbl->autofilename.endsWith(".xlsx"))
            tbl->autofilename.resize(tbl->autofilename.size()-5);
        tbl->autofilename += "_";
        tbl->autofilename += QVariant(it).toString();
        tbl->autofilename += ".xlsx";
        tbl->autosave = true;
        tbl->show();
        thrnum++;
        tbl->conName = QVariant(thrnum).toString();
        tbl->runSqlAsync(QVariant(thrnum).toString(), ui->comboBox->currentText(), ui->lineEdit_2->text(),  ui->lineEdit_3->text(),true,true);
        tbl->show();
    }
}

void Table::OpenNewWindow()
{
    QString str = cd->toPlainText();
    Table* tbl = new Table();
    tbl->sqlCode = "";
    tbl->cd->setPlainText("");
    tbl->show();
    thrnum++;
    tbl->conName = QVariant(thrnum).toString();
    tbl->runSqlAsync(QVariant(thrnum).toString(), ui->comboBox->currentText(), ui->lineEdit_2->text(),  ui->lineEdit_3->text(),true);
    tbl->show();
}

void Table::ShowIterateWindow()
{
    showIteratorWindow = !showIteratorWindow;
    if(showIteratorWindow)
    {
        iw.sb1.hide();
        iw.sb2.hide();
        iw.sb3.hide();
        iw.lbls1.hide();
        iw.lbls2.hide();
        iw.lbls3.hide();
        iw.nameline.hide();
        iw.button.hide();
    }
    else
    {
        iw.sb1.show();
        iw.sb2.show();
        iw.sb3.show();
        iw.lbls1.show();
        iw.lbls2.show();
        iw.lbls3.show();
        iw.nameline.show();
        iw.button.show();
    }
}

void Table::updatesuggestion()
{
    ui->SuggestLabel->setText(cd->lastSuggestedWord);
}

void Table::FillSuggest()
{
    cd->FillsuggestName();
}

void Table::OpenFile()
{
    QString str= "";
    str+= ui->lineEdit->text();
    qDebug()<<"opening:" << str;
    if(QDesktopServices::openUrl(QUrl::fromLocalFile(str)))
        ui->statuslabel->setText("Opening file.");
    else
        ui->statuslabel->setText("error opening file.");

}

void Table::UpdateTable()
{
    tableDataMutex.lock();
    ui->statuslabel->setText("updating table data...");
    ui->tableWidget->clear();
    ui->tableWidget->setColumnCount(headers.size());
    ui->tableWidget->setRowCount(tableData.size());
    ui->tableWidget->setHorizontalHeaderLabels(headers);

    for(int i=0;i<tableData.size();i++)
    {
        for (int a=0; a<tableData[i].size();a++)
        {
            QTableWidgetItem *item = new QTableWidgetItem(tableData[i][a].toString(),tableData[i][a].typeId());
            ui->tableWidget->setItem(i, a, item);
        }
    }
    ui->statuslabel->setText("table updated.");
    tableDataMutex.unlock();
    if(autosave)
    {
        ui->lineEdit->setText(autofilename);

        SaveToFile();
    }
}



void Table::ThreadReady()
{
    ui->statuslabel->setText("sql success.");
    UpdateTable();
}
void Table::exec()
{
    tableDataMutex.lock();
    if(!tdb.isOpen())
    {
        bool ok = tdb.open();
        if(ok)
            qDebug() << "db opened";
        else
            qDebug() << "nope: "<< tdb.lastError().text().toStdString();
    }
    QString str = sqlCode;
    QSqlQuery q(str,tdb);
    headers.clear();


    if(q.exec(str))
    {
        qDebug() << "query sucess";
        if(closing)
            return;

        for(int a=0,total = q.record().count(); a<total;a++)
            headers<<q.record().fieldName(a);
        int i=0;

        tableData.clear();
        while(q.next())
        {
            if(closing)
                return;
            tableData.emplace_back();
            for(int a=0,total = q.record().count(); a<total;a++)
            {
                tableData.back().push_back(q.value(a));
            }
            i++;
            //qDebug() << "wrk"<<i;
        }
    }
    else
    {
        headers.clear();
        tableData.clear();
        tableData.emplace_back();
        headers.push_back("Error");
        headers.push_back("db Error");
        headers.push_back("driver Error");
        qDebug() << "Error in sql query:"<< q.lastError().text().toStdString().c_str();
        qDebug() << "Error from database:"<< tr(q.lastError().databaseText().toStdString().c_str());
        qDebug() << "Error from driver:"<< q.lastError().driverText();

        tableData.back().push_back(q.lastError().text().toStdString().c_str());
        tableData.back().push_back(q.lastError().databaseText());
        tableData.back().push_back(q.lastError().driverText());
    }
    q.clear();
    q.finish();
    qDebug() << "Execd";
    tableDataMutex.unlock();
    emit execdReady();

}

Table::~Table()
{
    qDebug()<<"Connection closed"<<tdb.connectionName();
    QSqlDatabase::removeDatabase(tdb.connectionName());
    delete ui;
}

void Table::closeEvent(QCloseEvent* ev)
{
    closing = true;
    qDebug()<<"Connection closed"<<tdb.connectionName();
    QSqlDatabase::removeDatabase(tdb.connectionName());
}

void Table::on_pushButton_2_clicked()
{
    SaveToFile();
}
void Table::SaveToFile()
{
    ui->statuslabel->setText("saving...");
    QXlsx::Document xlsxR3;

    qDebug() << "saving";
    qDebug() << "ui->tableWidget->rowCount()"<<ui->tableWidget->rowCount();
    qDebug() << "ui->tableWidget->columnCount()"<<ui->tableWidget->columnCount();


    for(int i=0;i<ui->tableWidget->columnCount();i++)
    {
        QVariant var = ui->tableWidget->horizontalHeaderItem(i)->text();

        xlsxR3.write(1,i+1,var);
    }
    for(int i=0;i<tableData.size();i++)
        for(int a=0;a<tableData[i].size();a++)
        {
            int row =i+2;
            int column =a+1;
            xlsxR3.write(row,column,tableData[i][a]);
        }

    xlsxR3.addSheet("SQL");
    xlsxR3.selectSheet("SQL");
    xlsxR3.write(1,1,"SQL QUERY");
    xlsxR3.write(2,1,sqlCode);
    xlsxR3.selectSheet("Sheet1");
    QString str = "";
    str+=ui->lineEdit->text();
    qDebug()<< str;
    if(xlsxR3.saveAs(str))
        ui->statuslabel->setText("saved.");
    else
        ui->statuslabel->setText("failed to save, file probably opened.");

}

void Table::connectDB(QString conname,QString dbname,QString usrname, QString password)
{
    tdb.close();
    tdb = QSqlDatabase::addDatabase("QODBC",conname);
    tdb.setDatabaseName(dbname);
    tdb.setUserName(usrname);
    tdb.setPassword(password);
    bool ok = tdb.open();
    if(ok)
    {
        qDebug() << "db opened";

        ui->comboBox->setCurrentText(dbname);
        ui->lineEdit_2->setText(usrname);
        ui->lineEdit_3->setText(password);

        userDS.Load("userdata.txt");
        userDS.GetObject(ui->comboBox->currentText().toStdString())["name"] = ui->lineEdit_2->text().toStdString();
        userDS.GetObject(ui->comboBox->currentText().toStdString())["password"] = ui->lineEdit_3->text().toStdString();

        userDS.data["User"]["lastDBName"] = ui->comboBox->currentText().toStdString();
        userDS.data["User"]["name"] = ui->lineEdit_2->text().toStdString();
        userDS.data["User"]["password"] = ui->lineEdit_3->text().toStdString();

        userDS.Save("userdata.txt");
        if(LastDbName != dbname)
        {
            cd->highlighter->UpdateTableColumns(&tdb);

            cd->setPlainText(cd->toPlainText());
            cd->setPlainText(cd->toPlainText());
        }
        LastDbName = dbname;

    }
    else
        qDebug() << "nope: "<< tdb.lastError().text().toStdString();
}

void Table::on_pushButton_clicked()
{
    connectDB("", ui->comboBox->currentText(),ui->lineEdit_2->text(),ui->lineEdit_3->text());
}


void Func(Table* tbl)
{
    tbl->exec();
    return;
}

void  Table::runSqlAsync(QString conname,QString dbname,QString usrname, QString password, bool createconnection, bool runall)
{
    ui->statuslabel->setText("running sql...");
    if(cd->textCursor().selectedText().size() <= 5)
    {
        sqlCode = "";
        QString text = cd->toPlainText();
        int start = cd->textCursor().position()-1;
        while(start>0 && text[start]!=';')
            start--;
        if(text[start]==';')
            start++;
        int iter = start;
        sqlCode.push_back(text[iter]);
        iter++;
        while(iter<text.size())
        {
            sqlCode.push_back(text[iter]);
            if((text[iter]==';'))
                break;
            iter++;
        }
        qDebug()<<"sql start "<<start;
        qDebug()<<"sql start "<<iter-1;
    }
    else
        sqlCode = cd->textCursor().selectedText();

    if(runall)
        sqlCode = cd->toPlainText();
    //qDebug()<<"sql"<<sqlCode;

    QString str = "sqlBackup/";
    str += QTime::currentTime().toString();
    str +=".sql";
    str.replace(":"," ");
    qDebug()<<str;
    std::ofstream stream (str.toStdString());
    stream << cd->toPlainText().toStdString();
    stream.close();

    std::ofstream stream2 ("stock.sql");
    stream2 << cd->toPlainText().toStdString();
    stream2.close();


    userDS.Load("userdata.txt");
    userDS.GetObject("User")["lastDBName"] = ui->comboBox->currentText().toStdString();
    userDS.GetObject("User")["name"] = ui->lineEdit_2->text().toStdString();
    userDS.GetObject("User")["password"] = ui->lineEdit_3->text().toStdString();

    userDS.GetObject(ui->comboBox->currentText().toStdString())["name"] = ui->lineEdit_2->text().toStdString();
    userDS.GetObject(ui->comboBox->currentText().toStdString())["password"] = ui->lineEdit_3->text().toStdString();

    userDS.Save("userdata.txt");

    if(createconnection)
    {
        connectDB(conname,dbname,usrname,password);
    }


    if(thr!=nullptr)
        thr->terminate();

    thr = QThread::create(Func,this);
    thr->start();
}


void Table::on_pushButton_3_clicked()
{
    runSqlAsync(conName , ui->comboBox->currentText(),ui->lineEdit_2->text(),ui->lineEdit_3->text(), true);

}


void Table::on_comboBox_currentTextChanged(const QString &arg1)
{
    userDS.Load("userdata.txt");

    ui->lineEdit_2->setText(userDS.GetProperty(ui->comboBox->currentText().toStdString(),"name").c_str());
    ui->lineEdit_3->setText(userDS.GetProperty(ui->comboBox->currentText().toStdString(),"password").c_str());

    userDS.Save("userdata.txt");
}


