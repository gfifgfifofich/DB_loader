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
#include <QSqlDriver>


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
        strl.push_back((x.first + " " +  x.second).c_str());
        strl.back() = strl.back().trimmed();
    }
    ui->comboBox->addItems(strl);
    ui->comboBox->setCurrentText(userDS.GetProperty("User","lastDBName").c_str());
    strl.clear();
    for( auto x : userDS.data["UserDrivers"])
    {
        strl.push_back((x.second).c_str());
        strl.back() = strl.back().trimmed();
    }
    ui->comboBoxDriver->addItems(strl);
    ui->comboBoxDriver->setCurrentText(userDS.GetProperty("User","lastDriver").c_str());

    if((userDS.GetPropertyAsBool("Flags","Postgre")))
        ui->checkBox->setCheckState( Qt::CheckState::Checked);
    else
        ui->checkBox->setCheckState( Qt::CheckState::Unchecked);
    if((userDS.GetPropertyAsBool("Flags","Script")))
        ui->CheckBoxScript->setCheckState( Qt::CheckState::Checked);
    else
        ui->CheckBoxScript->setCheckState( Qt::CheckState::Unchecked);




    userDS.Save("userdata.txt");



    connectDB(conName ,ui->comboBoxDriver->currentText(), ui->comboBox->currentText(),ui->lineEdit_2->text(),ui->lineEdit_3->text());


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
        tbl->autosaveXLSX = true;
        tbl->show();
        thrnum++;
        tbl->conName = QVariant(thrnum).toString();
        tbl->runSqlAsync(QVariant(thrnum).toString(), ui->comboBoxDriver->currentText(),ui->comboBox->currentText(), ui->lineEdit_2->text(),  ui->lineEdit_3->text(),true,true);
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
    tbl->runSqlAsync(QVariant(thrnum).toString(),ui->comboBoxDriver->currentText(), ui->comboBox->currentText(), ui->lineEdit_2->text(),  ui->lineEdit_3->text(),true);
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
    if(autosaveXLSX)
    {
        ui->lineEdit->setText(autofilename);

        SaveToFile();
    }

    if(autosaveSQLITE)
    {

        ui->lineEdit->setText(autofilename);

        on_SaveToSQLiteTable_clicked();
    }
    emit TableUpdated();
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

    q.setForwardOnly(true);
    q.prepare(str);
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
    executing = false;
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

void Table::connectDB(QString conname,QString driver, QString dbname,QString usrname, QString password)
{
    tdb.close();
    bool sqlite = false;
    QString dbSchemaName = "NoName";
    if(driver.trimmed() != "LOCAL_SQLITE_DB")
        tdb = QSqlDatabase::addDatabase("QODBC",conname);
    else
    {
        tdb = QSqlDatabase::addDatabase("QSQLITE",conname);
        dbname = "SQLiteDB.db";
        dbSchemaName = "SQLiteDB";
        usrname = " ";
        password = " ";
        tdb.setDatabaseName("SQLiteDB.db");
        sqlite = true;
    }

    if(!sqlite){
    QString connectString = "Driver={";
    connectString.append(driver.trimmed()); // "Oracle in OraClient12Home1_32bit"
    connectString.append("};");
    if(!cd->highlighter->PostgresStyle)
    {
        connectString.append("DBQ=" );
        connectString.append(dbname.trimmed() );
        connectString.append(";");
        connectString.append("UID=");
        connectString.append(usrname.trimmed());
        connectString.append(";");
        connectString.append("PWD=");
        connectString.append(password.trimmed());
        connectString.append(";");


        QStringList strl = connectString.trimmed().split(';');
        QString filename;
        if(strl.size() > 1)
        {
            strl = strl[1].split('/');
            if(strl.size() > 1)
                dbSchemaName = strl[1];
            else if(strl.size() > 0)
                dbSchemaName = strl[0];
        }

    }
    else
    {
        QString server;
        QString port;
        QString database;
        QStringList strl = dbname.trimmed().split(':');
        if(strl.size() > 0)
            server = strl[0];
        if(strl.size() > 1)
        {
            strl = strl[1].split('/');

            if(strl.size() > 0)
                port = strl[0];
            if(strl.size() > 1)
                database = strl[1];
            dbSchemaName = database;
        }

        connectString.append("Server=" );
        connectString.append(server.trimmed());
        connectString.append(";Port=" );
        connectString.append(port.trimmed());
        connectString.append(";Database=" );
        connectString.append(database.trimmed());
        connectString.append(";Uid=" );
        connectString.append(usrname.trimmed());
        connectString.append(";Pwd=" );
        connectString.append(password.trimmed());
        connectString.append(";" );
    }
    tdb.setDatabaseName(connectString);
    qDebug() << connectString;
    }
    cd->highlighter->QSLiteStyle = sqlite;
    bool ok = tdb.open();
    if(ok)
    {
        qDebug() << "db opened";

        ui->comboBox->setCurrentText(dbname);
        ui->lineEdit_2->setText(usrname);
        ui->lineEdit_3->setText(password);

        userDS.Load("userdata.txt");
        userDS.data["Flags"]["Postgre"] = "1";
        userDS.GetObject(ui->comboBox->currentText().toStdString())["name"] = ui->lineEdit_2->text().toStdString();
        userDS.GetObject(ui->comboBox->currentText().toStdString())["password"] = ui->lineEdit_3->text().toStdString();
        userDS.data["User"]["lastDriver"] = ui->comboBoxDriver->currentText().toStdString();
        userDS.data["User"]["lastDBName"] = ui->comboBox->currentText().toStdString();

        QString LastTmpDriverName =  ui->comboBoxDriver->currentText();
        QString LastTmpDbName = ui->comboBox->currentText();

        userDS.data["User"]["name"] = ui->lineEdit_2->text().toStdString();
        userDS.data["User"]["password"] = ui->lineEdit_3->text().toStdString();


        if(ui->checkBox->checkState()== Qt::Checked)
            userDS.data["Flags"]["Postgre"] = "1";
        else
            userDS.data["Flags"]["Postgre"] = "0";

        if(ui->CheckBoxScript->checkState()== Qt::Checked)
            userDS.data["Flags"]["Script"] = "1";
        else
            userDS.data["Flags"]["Script"] = "0";



        userDS.Save("userdata.txt");



        QStringList strl;
        userDS.data["UserDBs"][ui->comboBox->currentText().toStdString()];
        for( auto x : userDS.data["UserDBs"])
        {
            strl.push_back((x.first + " " + x.second).c_str());
            strl.back() = strl.back().trimmed();
        }
        ui->comboBox->clear();
        ui->comboBox->addItems(strl);

        strl.clear();

        for( auto x : userDS.data["UserDrivers"])
        {
            strl.push_back(x.second.c_str());
            strl.back() = strl.back().trimmed();
        }

        if(!strl.contains(ui->comboBoxDriver->currentText()))
            userDS.data["UserDrivers"][std::to_string(ui->comboBoxDriver->count())] = ui->comboBoxDriver->currentText().toStdString();
        ui->comboBoxDriver->clear();
        ui->comboBoxDriver->addItems(strl);

        ui->comboBox->setCurrentText(LastTmpDbName );
        ui->comboBoxDriver->setCurrentText( LastTmpDriverName);

        if(ui->checkBox->checkState()== Qt::Checked)
            userDS.data["Flags"]["Postgre"] = "1";
        else
            userDS.data["Flags"]["Postgre"] = "0";
        if(ui->CheckBoxScript->checkState()== Qt::Checked)
            userDS.data["Flags"]["Script"] = "1";
        else
            userDS.data["Flags"]["Script"] = "0";
        userDS.Save("userdata.txt");
        if(LastDbName != dbname)
        {
            cd->highlighter->UpdateTableColumns(&tdb,dbSchemaName);

            cd->setPlainText(cd->toPlainText());
            cd->setPlainText(cd->toPlainText());
        }
        LastDbName = dbname;
        LastDriverName = LastTmpDriverName;

    }
    else
        qDebug() << "nope: "<< tdb.lastError().text().toStdString();
}

void Table::on_pushButton_clicked()
{
    connectDB("", ui->comboBoxDriver->currentText(),ui->comboBox->currentText(),ui->lineEdit_2->text(),ui->lineEdit_3->text());
}


void Func(Table* tbl)
{
    tbl->exec();
    return;
}


inline int active_Windows =0;
inline bool waiting = false;
inline int waitongpos = 0;


void Table::subWindowDone()
{
    active_Windows -=1;
    qDebug()<<"active_Windows called: "<< active_Windows;
    if(active_Windows<0)
    {
        qDebug()<<"active_Windows count less than 0, something went wrong";
    }

    if(waiting && active_Windows <= 0)
    {
        waiting = false;
        qDebug()<<"resuming";
        RunAsScript(waitongpos);
    }

}


void Table::RunAsScript(int startfrom)
{
    userDS.data["Flags"]["Script"] = "0";
    ui->CheckBoxScript->setCheckState(Qt::Unchecked);

    QString text = cd->toPlainText();
    bool in_comand = false;
    bool in_Select = false;
    bool in_Set = false;
    bool in_XLSXSave = false;
    bool in_SQLITESave = false;
    bool prev_is_newline = true;
    bool newTokenAdded = false;
    bool newVariableAdded = false;
    bool in_brakets = false;
    bool WaitNextLine = false;
    bool readSqlUntillNextCommand = false;
    int state = 0;
    QStringList variables;
    QStringList set_variables;
    QString     save_text;

    QStringList tokens;
    QString lastSQL = "";
    for(int i=startfrom;i<text.size();i++)
    {

        if(WaitNextLine && text[i] != '\n')
        {
            continue;
        }
        else if (WaitNextLine && text[i] == '\n')
        {
            WaitNextLine = false;
            in_comand = false;

            continue;// next line reached
        }
        if(readSqlUntillNextCommand)
            lastSQL.push_back(text[i]);

        if(newVariableAdded && (in_XLSXSave || in_SQLITESave) && in_Select)
        {
            qDebug()<<"";
            qDebug()<<"SelectDB Comand";
            if(variables.size()<4)
                qDebug() << "less than 4 variables in selectDB";
            else
                qDebug()<<variables;
            qDebug()<<lastSQL;
            qDebug()<<"";
            qDebug()<<"SaveToXLSX";
            qDebug()<<save_text;
            qDebug()<<"";
            qDebug()<<i;
            qDebug()<<text.size();
            qDebug()<<"";

            QString str = cd->toPlainText();
            Table* tbl = new Table();
            tbl->sqlCode = lastSQL;
            tbl->cd->setPlainText(lastSQL);

            active_Windows+=1;
            tbl->autofilename = save_text;
            tbl->autosaveXLSX = in_XLSXSave;
            tbl->autosaveSQLITE = in_SQLITESave;
            tbl->show();
            thrnum++;
            tbl->conName = QVariant(thrnum).toString();
            tbl->connectDB(QVariant(thrnum).toString(), variables[0].trimmed(),variables[1].trimmed(), variables[2].trimmed(),  variables[3].trimmed());
            tbl->runSqlAsync(QVariant(thrnum).toString(), variables[0].trimmed(),variables[1].trimmed(), variables[2].trimmed(),  variables[3].trimmed(),true,true);

            tbl->show();

            connect( tbl, SIGNAL(TableUpdated()), this, SLOT(subWindowDone()), Qt::QueuedConnection );




            in_comand = false;
            in_Select = false;
            in_Set = false;
            in_XLSXSave = false;
            in_SQLITESave = false;
            prev_is_newline = true;
            newTokenAdded = false;
            newVariableAdded = false;
            in_brakets = false;
            WaitNextLine = false;
            readSqlUntillNextCommand = false;
            state = 0;
            variables.clear();
            set_variables.clear();
            tokens.clear();
            save_text = "";
            lastSQL = "";

        }
        if(text[i]=='\n' && !in_Select && !in_XLSXSave && !in_SQLITESave)
        {// reset
            in_comand = false;
            in_Select = false;
            in_Set = false;
            in_XLSXSave = false;
            in_SQLITESave = false;
            prev_is_newline = false;
            newTokenAdded = false;
            newVariableAdded = false;
            in_brakets = false;
            WaitNextLine = false;
            readSqlUntillNextCommand = false;
            state = 0;
            variables.clear();
            tokens.clear();
            state = 0;
            continue;
        }
        if( i+1 < text.size() && text[i] == '-' && text[i+1] == '-')
        {
            i++;
            in_comand = true;
            continue;
        }
        if(in_comand)
        {

            if(readSqlUntillNextCommand)
            {
                while(lastSQL.back() !=';')
                    lastSQL.resize(lastSQL.size()-1);
            }
            readSqlUntillNextCommand = false;

            if(in_brakets)
            {
                variables.back().push_back(text[i]);
            }

            if(newVariableAdded)
            {
                if(in_Select && variables.size() == 4)
                {
                    // nextline, sql
                    //WaitNextLine = true;
                    readSqlUntillNextCommand = true;
                    in_comand = false;

                }
            }


            if(tokens.size() <=0)
            {
                tokens.push_back("");
            }
            if(text[i] == '{')
            {
                variables.push_back("");
                in_brakets = true;
                qDebug()<<"in_brakets";
                continue;
            }
            if(text[i] == '}')
            {
                variables.back().resize(variables.back().size()-1);
                if(in_SQLITESave || in_XLSXSave)
                {
                    save_text = variables.back();
                }
                in_brakets = false;
                newVariableAdded = true;
                qDebug()<<"variable " << variables.back();
                continue;
            }
            if(text[i] == ' ')
            {
                qDebug()<<"tokens " << tokens.back();
                newTokenAdded = true;
            }
            if(newTokenAdded)
            {

                qDebug()<<"Processing token " << tokens.back().trimmed();
                if(tokens.back().trimmed() == "SelectDB")
                {
                    in_Select = true;
                    lastSQL = "";
                    qDebug()<<"in SelectDB";
                }
                if(tokens.back().trimmed() == "Set")
                    in_Set = true;
                if(in_Select && tokens.back().trimmed() == "XLSXSave")
                {
                    in_XLSXSave = true;
                    qDebug()<<"in XLSXSave";

                }
                if(in_Select && tokens.back().trimmed() == "SQLITESave")
                    in_SQLITESave = true;
                if(tokens.back().trimmed() == "Wait" && active_Windows > 0)
                {
                    waiting = true;
                    waitongpos = i;
                    qDebug()<<"Waiting ";
                    return;
                }
                state = 1;
                tokens.push_back("");

            }
            tokens.back().push_back(text[i]);

        }

        newTokenAdded = false;
        prev_is_newline = false;
        newVariableAdded = false;
        if(text[i] == '\n')
            prev_is_newline =true;
    }
    waitongpos = 0;
    userDS.Load("userdata.txt");
    userDS.data["Flags"]["Script"] = "1";
    userDS.Save("userdata.txt");

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

}


void  Table::runSqlAsync(QString conname,QString driver,QString dbname,QString usrname, QString password, bool createconnection, bool runall)
{

    userDS.Load("userdata.txt");
    userDS.GetObject("User")["lastDriver"] = ui->comboBoxDriver->currentText().toStdString();
    userDS.GetObject("User")["lastDBName"] = ui->comboBox->currentText().toStdString();
    userDS.GetObject("User")["name"] = ui->lineEdit_2->text().toStdString();
    userDS.GetObject("User")["password"] = ui->lineEdit_3->text().toStdString();

    userDS.GetObject(ui->comboBox->currentText().toStdString())["name"] = ui->lineEdit_2->text().toStdString();
    userDS.GetObject(ui->comboBox->currentText().toStdString())["password"] = ui->lineEdit_3->text().toStdString();

    if(ui->checkBox->checkState() == Qt::Checked)
        userDS.data["Flags"]["Postgre"] = "1";
    else
        userDS.data["Flags"]["Postgre"] = "0";
    if(ui->CheckBoxScript->checkState()== Qt::Checked)
        userDS.data["Flags"]["Script"] = "1";
    else
        userDS.data["Flags"]["Script"] = "0";

    userDS.Save("userdata.txt");

    if(executing)
        return;



    if(ui->CheckBoxScript->checkState() != Qt::Checked || runall)
    {

        executing = true;
        ui->statuslabel->setText("running sql...");
        int sqlstart = 0;
        int sqlend = cd->toPlainText().size();
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
            sqlstart = start;
            sqlCode.push_back(text[iter]);
            iter++;
            while(iter<text.size())
            {
                sqlCode.push_back(text[iter]);
                sqlend = start;
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
        {
            sqlCode = cd->toPlainText();
        }//qDebug()<<"sql"<<sqlCode;

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




        if(createconnection)
        {
            connectDB(conname,driver,dbname,usrname,password);
        }
        QTextCursor cursor = cd->textCursor();
        cursor.setPosition(sqlstart, QTextCursor::MoveAnchor);
        cursor.setPosition(sqlend+1, QTextCursor::KeepAnchor);

        if(thr!=nullptr)
            thr->terminate();

        thr = QThread::create(Func,this);
        thr->start();
    }
    else
    {
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
        RunAsScript();
    }
}


void Table::on_pushButton_3_clicked()
{
    runSqlAsync(conName ,ui->comboBoxDriver->currentText(), ui->comboBox->currentText(),ui->lineEdit_2->text(),ui->lineEdit_3->text(), true);

}


void Table::on_comboBox_currentTextChanged(const QString &arg1)
{
    userDS.Load("userdata.txt");

    ui->lineEdit_2->setText(userDS.GetProperty(ui->comboBox->currentText().toStdString(),"name").c_str());
    ui->lineEdit_3->setText(userDS.GetProperty(ui->comboBox->currentText().toStdString(),"password").c_str());

    userDS.Save("userdata.txt");
}



void Table::on_checkBox_checkStateChanged(const Qt::CheckState &arg1)
{
    cd->highlighter->PostgresStyle = arg1;
}




void Table::on_SaveToSQLiteTable_clicked()
{// everything has SQLITE_ prefix to ensure no queries will run on main db's

    QSqlDatabase tmpdb = QSqlDatabase::addDatabase("QSQLITE","SQLITE db connection");
    tmpdb.setDatabaseName("SQLiteDB.db");
    tmpdb.open();
    QSqlQuery SQLITE_q(tmpdb);


    QString SQLITE_sql = "Drop table ";
    SQLITE_sql += ui->lineEdit->text();
    SQLITE_sql += ";";
    if(SQLITE_q.exec(SQLITE_sql))
        qDebug()<< "Dropped sqlite table";
    else
        qDebug()<< "Failed to save to sqlite: " <<SQLITE_q.lastError().text();

    SQLITE_sql = "Create table ";
    SQLITE_sql += ui->lineEdit->text();

    SQLITE_sql += " ( ";
    for(int i=0;i<headers.size();i++)
    {
        SQLITE_sql += headers[i];

        SQLITE_sql += " text ";

        if(i+1<headers.size())
            SQLITE_sql += ", ";
    }
    SQLITE_sql += " ); ";
    qDebug()<< SQLITE_sql;

    if(SQLITE_q.exec(SQLITE_sql))
        qDebug()<< "Created sqlite table";
    else
        qDebug()<< "Failed to save to sqlite: " <<SQLITE_q.lastError().text();


    /* form a string
     *
      INSERT INTO 'tablename'
          SELECT 'data1' AS 'column1', 'data2' AS 'column2'
UNION ALL SELECT 'data1', 'data2'
UNION ALL SELECT 'data1', 'data2'

     */

    SQLITE_sql = "Insert into ";
    SQLITE_sql += ui->lineEdit->text();
    SQLITE_sql += " Select ";


    if(tableData.size()>0)
    {
        bool first = true;
        for(int a=0;a<tableData[0].size();a++)
        {
            if(!first)
                SQLITE_sql += ",";
            first = false;
            SQLITE_sql += " '";
            SQLITE_sql += tableData[0][a].toString();
            SQLITE_sql += "' ";
            SQLITE_sql += " as ";
            SQLITE_sql += " '";
            SQLITE_sql += headers[a];
            SQLITE_sql += "' ";
        }
        //SQLITE_sql += '\n';
    }


    int lasti=0;
    for(int i=1;i<tableData.size();i++)
    {

        SQLITE_sql += " union all Select ";
        bool first = true;
        for(int a=0;a<tableData[i].size();a++)
        {
            if(!first)
                SQLITE_sql += ",";
            first = false;
            int row =i+2;
            int column =a+1;
            bool is_text = false;
            tableData[i][a].toDouble(&is_text);
            is_text = !is_text;

            //if(is_text)

            SQLITE_sql += " '";
            SQLITE_sql += tableData[i][a].toString();
            SQLITE_sql += "' ";

            //if(is_text)
        }
        if(i - lasti > 300)
        {
            lasti = i+1;
            if(!SQLITE_q.exec (SQLITE_sql))
                qDebug()<< "Failed to save to sqlite: " <<SQLITE_q.lastError().text();

            SQLITE_sql = "Insert into ";
            SQLITE_sql += ui->lineEdit->text();
            SQLITE_sql += " Select ";


            if(tableData.size()>lasti)
            {
                bool first = true;
                for(int a=0;a<tableData[lasti].size();a++)
                {
                    if(!first)
                        SQLITE_sql += ",";
                    first = false;
                    SQLITE_sql += " '";
                    SQLITE_sql += tableData[lasti][a].toString();
                    SQLITE_sql += "' ";
                    SQLITE_sql += " as ";
                    SQLITE_sql += " '";
                    SQLITE_sql += headers[a];
                    SQLITE_sql += "' ";
                }
                //SQLITE_sql += '\n';
            }

            i+=1;
        }

    }
    //qDebug() << SQLITE_sql;
    if(!SQLITE_q.exec (SQLITE_sql))
        qDebug()<< "Failed to save to sqlite: " <<SQLITE_q.lastError().text();

    tmpdb.close();
}

