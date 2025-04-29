#include "loaderwidnow.h"
#include "ui_loaderwidnow.h"
#include <qbarset.h>
#include <qclipboard.h>
#include <qdir.h>
#include <qpdfwriter.h>
#include <qrandom.h>
#include <qshortcut.h>
#include <fstream>
#include <qsqldriver.h>
#include <qsqlerror.h>
#include <QDesktopServices>
#include <QFileDialog>
#include "replacedialog.h"
#include <QQmlApplicationEngine>
#include <qtimer.h>
#include "tokenprocessor.h"
/*
+                                          add togglable "add db name into file name" // feature added, not togglable
+                                          add ability to stop downloading query at any point of downloading, mb togglable autopause at 500
+                                          save only driver and db name in workspaces, fetch the rest from userdata, its safer
+                                          themes from userdata
+                                          Replace window - replaces every "string" with other "string", yea
+                                          Highlight from bracket to bracket with codeEditor HighLight selection (will highlight background from bracket to bracket)
+                                          Highlighting of selected token
+ testing                                  fix subtables
+ constant correction                      datatypes
+                                          tab press reg
+                                          Timer
+- reimplement Patterns compleatly through .ds files

+ open files through cmd
- Scroll code preview


+ SubexecToArray{
        --{driver} {database}
        select * from stufff
        }
+ SubexecToMagic{
        --{driver} {database}
        select * from stufff
        }
+ SubexecToUnionAllTable{
        --{driver} {database}
        select * from stufff
        }


+- Iterate magic/arrays by 10k each -- possible through regular forloops

+ ForLoop{
        --{value} {opt1,opt2,opt3}
        select * from stufff where value
        }
+ ForLoop{
        --{value} {from} {to} {optional_step = 1}
        select * from stufff where value
        }
+ SubexecTo{Silent}{SqliteTable, ExcelTable, CSV} some method of updating sqlite tables in sqlite query


- add special highlighting for unaccessible tables?
    select * from dba_tables where lower(owner) like '%%';
    select * from dba_TAB_COLUMNS where lower(table_name) like '%%';
    dba_ - all, all_ - user has access




*/


inline QString usrDir;
inline QString documentsDir;
inline DataStorage userDS;
inline DataStorage historyDS;
inline QString appfilename;
inline int thrnum;
inline QTime lastMultiRunPress = QTime::currentTime();

QStringList allPosibbleTokens;
QMap<QString,int> allPosibbleTokensMap;

LoaderWidnow::LoaderWidnow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::LoaderWidnow)
{
    ui->setupUi(this);
    ui->stopLoadingQueryButton->hide();


    cd = new CodeEditor();
    ui->CodeEditorLayout->addWidget(cd);

    //open new app instance
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_N), this, SLOT(OpenNewWindow()));

    // run query
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_R), this, SLOT(runSqlAsync()));
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Return), this, SLOT(runSqlAsync()));

    //.xlsx export
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_E), this, SLOT(on_SaveXLSXButton_pressed())); // save to file
    // open .xlsx file
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_O), this, SLOT(OpenFile()));

    // save workspace
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_S), this, SLOT(SaveWorkspace()));

    // replace code
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_F), this, SLOT(replaceTool()));
    // comment code
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_B), cd, SLOT(CommentSelected()));

    // copy from tableview
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_C), this, SLOT(CopySelectionFormTable()));
    new QShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_C), this, SLOT(CopySelectionFormTableSql()));

    // switching different windows
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_G), this, SLOT(ShowGraph()));
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_H), this, SLOT(ShowHistoryWindow()));
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_W), this, SLOT(ShowWorkspacesWindow()));

    // graph window init
    gw.Init();
    //ui->CodeEditorLayout->addLayout(&iw.iter_layout);
    ui->CodeEditorLayout->addLayout(&gw.graph_layout);
    ui->listWidget->hide();



    //signal binding
    //query states
    connect( &dc, SIGNAL(queryBeginCreating()), this, SLOT(onQueryBeginCreating()), Qt::QueuedConnection );
    connect( &dc, SIGNAL(queryBeginExecuting()), this, SLOT(onQueryBegin()), Qt::QueuedConnection );
    connect( &dc, SIGNAL(querySuccess()), this, SLOT(onQuerySuccess()), Qt::QueuedConnection );
    connect( &dc, SIGNAL(execedSql()), this, SLOT(UpdateTable()), Qt::QueuedConnection );
    connect(&executionTimer, SIGNAL(timeout()), this, SLOT(executionTimerTimeout()));
    //codeeditor
    connect( cd, SIGNAL(s_suggestedName()), this, SLOT(updatesuggestion()), Qt::QueuedConnection );
    //graph
    connect( &gw.buildGraphButton, SIGNAL(pressed()), this, SLOT(UpdateGraph()), Qt::QueuedConnection );
    connect( &gw.saveAsPDFButton, SIGNAL(pressed()), this, SLOT(saveGraphAsPDF()), Qt::QueuedConnection );
    connect( &gw.groupBysb, SIGNAL(valueChanged(int)), this, SLOT(on_graph_group_change(int)), Qt::QueuedConnection );
    connect( &gw.separateBysb, SIGNAL(valueChanged(int)), this, SLOT(on_graph_separator_change(int)), Qt::QueuedConnection );
    connect( &gw.dataColumnsb, SIGNAL(valueChanged(int)), this, SLOT(on_graph_data_change(int)), Qt::QueuedConnection );

    //maximize code editor
    ui->splitter->setSizes({1,2000,1});

    // load last database/driver
    if(userDS.Load((documentsDir + "/userdata.txt").toStdString()))
    {
        QStringList strl;
        for( auto x : userDS.data["UserDBs"])
        {
            strl.push_back((x.first + " " +  x.second).c_str());
            strl.back() = strl.back().trimmed();
        }
        ui->DBNameComboBox->addItems(strl);
        ui->DBNameComboBox->setCurrentText(userDS.GetProperty("User","lastDBName").c_str());
        strl.clear();
        for( auto x : userDS.data["UserDrivers"])
        {
            strl.push_back((x.second).c_str());
            strl.back() = strl.back().trimmed();
        }
        ui->driverComboBox->addItems(strl);
        ui->driverComboBox->setCurrentText(userDS.GetProperty("User","lastDriver").c_str());
        userDS.Save("userdata.txt");
    }

    // if program was opened from command line with filename to open
    if(launchOpenFile)
    {
        QFile file(launchOpenFileName);
        if (file.open(QFile::ReadOnly | QFile::Text))
            cd->setPlainText(file.readAll());
        LastWorkspaceName = launchOpenFileName;
        ui->workspaceLineEdit->setText(LastWorkspaceName);
        ui->splitter->setSizes({0,2000,0});
    }
    else
    {
        if(!LastWorkspaceName.endsWith(".sql"))
            LastWorkspaceName+=".sql";
        QFile file(documentsDir + "/" +"workspaces/" + LastWorkspaceName);
        if (file.open(QFile::ReadOnly | QFile::Text))
            cd->setPlainText(file.readAll());
    }

    // get database from file header
    QString text = cd->toPlainText();
    if(text.startsWith("-- {"))
    {
        QStringList tokens;
        int i = 0;
        bool inbrakets = false;
        while(i < text.size())
        {
            if(text[i] == '{')
            {
                tokens.push_back("");
                inbrakets = true;
                i++;
                continue;
            }
            if(text[i] == '}')
            {
                tokens.back() = tokens.back().trimmed();
                inbrakets = true;
                if(tokens.size() == 2)
                    break;
                i++;
                continue;
            }
            if(inbrakets)
            {
                tokens.back().push_back(text[i]);
            }
            i++;

        }
        if(tokens.size() >= 2)
        {
            ui->driverComboBox->setCurrentText(tokens[0].trimmed());
            ui->DBNameComboBox->setCurrentText(tokens[1].trimmed());

        }
    }
    QString username = userDS.GetObject( ui->DBNameComboBox->currentText().toStdString())["name"].c_str();
    QString password = userDS.GetObject(ui->DBNameComboBox->currentText().toStdString())["password"].c_str();

    // set focus on code editor
    cd->setFocus();

    //load tokenprocessor data
    if(!tokenProcessor.ds.Load((documentsDir + "/" +"FrequencyMaps/test.txt").toStdString()))
        qDebug() << "failed to load FrequencyMap: " <<  (documentsDir + "/" +"FrequencyMaps/test.txt").toStdString();

    // connect to database
    on_ConnectButton_pressed();




    // testing:

    DataStorage tmptokends;
    tmptokends.Load((documentsDir + "/FrequencyMaps/tokens.txt").toStdString());

    for(auto s : tmptokends.data)
    {
        for(auto a :s.second)
        {
            allPosibbleTokens.push_back(QString().fromLocal8Bit(a.second.c_str()).toLower().trimmed());
            allPosibbleTokensMap[allPosibbleTokens.back().toLower().trimmed()] = allPosibbleTokens.size()-1;
        }
    }
    qDebug()<<"loaded " << allPosibbleTokens.size() << " distinct tokens";

    int arch[4] = {1,70,70,1};
    allPosibbleTokens.size();
    arch[0] = allPosibbleTokens.size();
    arch[3] = allPosibbleTokens.size();
    nn.Create(arch,4);
    qDebug()<<"created nn {" << arch[0] << ", " << arch[1] ;//<< ", " << arch[2] << ", " << arch[3] << ", " << arch[4] << "}";
    nn.Randomize();
    nn.Randomize();
    qDebug()<<"Randomized";
    nn.lastCost = 10000000;

    //int arch[7] = {1,10,10,10,10,10,1};

    //nn.Create(arch,7);
    //qDebug()<<"created nn {" << arch[0] << ", " << arch[1] << ", " << arch[2] << ", " << arch[3] ;//<< ", " << arch[4] << "}";
    //nn.Randomize();
    //nn.Randomize();
    //qDebug()<<"Randomized";
    //nn.lastCost = 1000000000;





}
void LoaderWidnow::Init()
{
    tokenProcessor.ds.Load((documentsDir + "/" +"FrequencyMaps/test.txt").toStdString());
}

LoaderWidnow::~LoaderWidnow()
{
    qDebug()<<"closing window";
    dc.stopRunning();
    dc.db.close();
    if(sqlexecThread != nullptr)
    {
        // a mess to have more chances of clearing db connection
        try
        {
            dc.stopNow = true;
            // stop query if loading.
            // if query is executing, thead will be left alive untill executed
            // still cant cancel query propperly
            dc.db.driver()->cancelQuery(); // in case it will be possible some day...
            dc.db.close();
            dc.db.driver()->close();
            if(dc.query != nullptr)
                dc.query->finish();
            sqlexecThread->terminate();

        } catch (...)
        {

            sqlexecThread->terminate();
        }
    }
    delete ui;
}

void LoaderWidnow::on_ConnectButton_pressed()
{
    qDebug()<<"on_ConnectButton_pressed()";


    if(!tokenProcessor.ds.Load((documentsDir + "/" +"FrequencyMaps/test.txt").toStdString()))
        qDebug() << "failed to load FrequencyMap: " <<  (documentsDir + "/" +"FrequencyMaps/test.txt").toStdString();

    QRandomGenerator64 gen;
    QString conname = QVariant(gen.generate()).toString();
    QString driver = ui->driverComboBox->currentText();
    QString dbname = ui->DBNameComboBox->currentText();
    QString usrname = ui->userNameLineEdit->text();
    QString password = ui->passwordLineEdit->text();
    dc.connectionName = conname;
    bool differentDB = false;
    if(dc.LastDBName != dbname || (driver == "LOCAL_SQLITE_DB"))
        differentDB = true;


    if(dc.Create(driver, dbname, usrname, password))
    {

        ui->driverComboBox->setCurrentText(dc.driver);
        ui->DBNameComboBox->setCurrentText(dc.dbname);
        ui->userNameLineEdit->setText(dc.usrname);
        ui->passwordLineEdit->setText(dc.password);
        ui->connectionStatusLabel_2->setText(QString("connected to ") + dc.dbname);

        if(userDS.Load("userdata.txt"))
        {
            QString LastTmpDriverName =  ui->driverComboBox->currentText();
            QString LastTmpDbName = ui->DBNameComboBox->currentText();

            userDS.data[ui->DBNameComboBox->currentText().toStdString()]["name"] = dc.usrname.toStdString();
            userDS.data[ui->DBNameComboBox->currentText().toStdString()]["password"] = dc.password.toStdString();
            userDS.data["User"]["lastDriver"] = dc.driver.toStdString();
            userDS.data["User"]["lastDBName"] = dc.dbname.toStdString();
            userDS.data["User"]["name"] = dc.usrname.toStdString();
            userDS.data["User"]["password"] = dc.password.toStdString();

            QStringList strl;
            userDS.data["UserDBs"][ui->DBNameComboBox->currentText().toStdString()];
            for( auto x : userDS.data["UserDBs"])
            {
                strl.push_back((x.first + " " + x.second).c_str());
                strl.back() = strl.back().trimmed();
            }
            ui->DBNameComboBox->clear();
            ui->DBNameComboBox->addItems(strl);

            strl.clear();

            for( auto x : userDS.data["UserDrivers"])
            {
                strl.push_back(x.second.c_str());
                strl.back() = strl.back().trimmed();
            }

            if(!strl.contains(ui->driverComboBox->currentText()))
                userDS.data["UserDrivers"][std::to_string(ui->driverComboBox->count())] = ui->driverComboBox->currentText().toStdString();
            ui->driverComboBox->clear();
            ui->driverComboBox->addItems(strl);

            ui->DBNameComboBox->setCurrentText(LastTmpDbName );
            ui->driverComboBox->setCurrentText( LastTmpDriverName);


            userDS.data[ui->DBNameComboBox->currentText().toStdString()]["name"] = dc.usrname.toStdString();
            userDS.data[ui->DBNameComboBox->currentText().toStdString()]["password"] = dc.password.toStdString();
            userDS.data["User"]["lastDriver"] = dc.driver.toStdString();
            userDS.data["User"]["lastDBName"] = dc.dbname.toStdString();
            userDS.data["User"]["name"] = dc.usrname.toStdString();
            userDS.data["User"]["password"] = dc.password.toStdString();

            userDS.Save("userdata.txt");
            qDebug()<<"Saved usedata.txt";
        }
        else
            qDebug()<<"Failed to open usedata.txt";
        if(differentDB)
        {
            QString str = "";
            if(dbname.split('/').size()>1)
                str = dbname.split('/')[1];
            else str =dbname.split('/')[0];

            cd->highlighter->QSLiteStyle = dc.sqlite;//(driver == "QSQLite") || (driver == "LOCAL_SQLITE_DB");
            cd->highlighter->PostgresStyle = dc.postgre;//(driver == "QPSQL");
            cd->highlighter->UpdateTableColumns(&dc.db,str);

            cd->highlighter->rehighlight();
            cd->highlighter->rehighlight();

        }
        ui->driverComboBox->setCurrentText(dc.driver);
        ui->DBNameComboBox->setCurrentText(dc.dbname);
        ui->userNameLineEdit->setText(dc.usrname);
        ui->passwordLineEdit->setText(dc.password);
    }
    else
        ui->connectionStatusLabel_2->setText("Not connected");

}

// function to give to thread to process query async
void _AsyncFunc(LoaderWidnow* loader)
{
    loader->dc.execSql();
}

void LoaderWidnow::runSqlAsync()
{
    qDebug()<<"RunSqlAsync()";

    ui->stopLoadingQueryButton->show();

    // save new lastOpenedDb
    if(userDS.Load((documentsDir + "/userdata.txt").toStdString()))
    {
        userDS.data["User"]["lastDriver"] = ui->driverComboBox->currentText().toStdString();
        userDS.data["User"]["lastDBName"] = ui->DBNameComboBox->currentText().toStdString();
        userDS.data["User"]["name"] = ui->userNameLineEdit->text().toStdString();
        userDS.data["User"]["password"] = ui->passwordLineEdit->text().toStdString();

        userDS.data[ui->DBNameComboBox->currentText().toStdString()]["name"] = ui->userNameLineEdit->text().toStdString();
        userDS.data[ui->DBNameComboBox->currentText().toStdString()]["password"] = ui->passwordLineEdit->text().toStdString();

        userDS.Save("userdata.txt");
    }

    if(dc.executing)
    {

        qDebug() << dc.db.driver()->cancelQuery();
        return;
    }
    ui->pushButton_3->hide();
    queryExecutionState = 0;
    QRandomGenerator64 gen; // random 64 bit connection name, to not close previous connections with 99.9% chance.
    QString conname = QVariant(gen.generate()).toString();
    QString driver = ui->driverComboBox->currentText();
    QString dbname = ui->DBNameComboBox->currentText();
    QString usrname = ui->userNameLineEdit->text();
    QString password = ui->passwordLineEdit->text();

    dc.executing = true;

    // cut part of code to execute
    int sqlstart = 0;
    int sqlend = cd->toPlainText().size();
    if(cd->textCursor().selectedText().size() <= 5)
    {
        dc.sqlCode = "";
        QString text = cd->toPlainText();
        int start = cd->textCursor().position()-1;
        if(start<0)
            start = 0;
        while(start>0 && text[start]!=';')
            start--;
        while(start< text.size() && !text[start].isLetterOrNumber() && text[start] != '-')
            start++;
        QTextCursor tc = cd->textCursor();
        tc.setPosition(start,QTextCursor::MoveAnchor);
        dc._code_start_line = tc.blockNumber();
        dc._code_start_pos = tc.position();
        int iter = start;
        sqlstart = start;
        dc.sqlCode.push_back(text[iter]);
        iter++;
        while(iter<text.size())
        {
            dc.sqlCode.push_back(text[iter]);
            sqlend = start;
            if((text[iter]==';'))
                break;
            iter++;
        }
        tc.setPosition(iter,QTextCursor::MoveAnchor);
        qDebug()<<"sql start "<<start;
        qDebug()<<"sql start "<<iter-1;
        while(dc.sqlCode.endsWith(';'))
            dc.sqlCode.resize(dc.sqlCode.size()-1);
    }
    else
    {
        dc.sqlCode = cd->textCursor().selectedText().toStdString().c_str();
        dc.sqlCode = dc.sqlCode.replace('\r','\n');
        dc.sqlCode = dc.sqlCode.replace(QChar(0x2029), QChar('\n'));

        QTextCursor tc = cd->textCursor();
        tc.setPosition(cd->textCursor().selectionStart(),QTextCursor::MoveAnchor);
        dc._code_start_line = tc.blockNumber();
        dc._code_start_pos = tc.position();
    }
    if(runall)
    {
        dc._code_start_line = 0;
        dc._code_start_pos = 0;
        dc.sqlCode = cd->toPlainText();
        while(dc.sqlCode.endsWith(';'))
            dc.sqlCode.resize(dc.sqlCode.size()-1);
    }


    qDebug()<<"Executing sql:";
    ui->miscStatusLabel->setText(QString("running sql subqueries... start: ") + dc.executionStart.toString());
    qDebug()<<dc.sqlCode;
    qDebug()<<"";

    // save sqlBacup
    QString str = "sqlBackup/";
    QDate dt = QDate::currentDate();
    str += QVariant(dt.year()).toString();
    str += "_";
    if(QVariant(dt.month() ).toString().size() <=1)
        str+="0";
    str += QVariant(dt.month()).toString();
    str += "_";
    if(QVariant(dt.day() ).toString().size() <=1)
        str+="0";
    str += QVariant(dt.day() ).toString();
    str += "_";
    str += QTime::currentTime().toString();
    str +=".sql";
    str.replace(":","_");
    qDebug()<<documentsDir + "/" +str;
    std::ofstream stream ((documentsDir + "/" +str).toStdString());
    stream << cd->toPlainText().toStdString();
    stream.close();
    SaveWorkspace();

    // move cursor to end of code, to highlight some user errors with missing ;
    QTextCursor cursor = cd->textCursor();
    cursor.setPosition(sqlstart, QTextCursor::MoveAnchor);
    cursor.setPosition(sqlend+1, QTextCursor::KeepAnchor);
    if(sqlexecThread!=nullptr)
        sqlexecThread->terminate();
    if(createconnection || !dc.db.isOpen() || dc.driver != ui->driverComboBox->currentText() ||dc.dbname != ui->DBNameComboBox->currentText())
    {
        dc.connectionName = conname;
        dc.driver = driver;
        dc.dbname = dbname;
        dc.usrname = usrname;
        dc.password = password;
        qDebug() << "creating connection";
        on_ConnectButton_pressed();
        qDebug() << "created connection";
    }


    ui->miscStatusLabel->setText("running sql...");
    sqlexecThread = QThread::create(_AsyncFunc,this);
    sqlexecThread->start();
    ui->miscStatusLabel->setText("running sql...");

    executionTimer.setSingleShot(false);
    executionTimer.setTimerType(Qt::CoarseTimer);
    executionTimer.setInterval(15);
    executionTimer.start();

}
void LoaderWidnow::IterButtonPressed()
{
    qDebug() << "depricated function";
}

void LoaderWidnow::executionTimerTimeout()
{// update label

    QString msg = "";
    if(queryExecutionState >=3)
    {
        msg += QVariant(dc.data.tbldata.size()).toString();
        msg += " : ";
        if(dc.data.tbldata.size()>0)
            msg += QVariant(dc.data.tbldata[0].size()).toString();
        else
            msg += "0";
    }
    dc.executionTime = QDateTime::currentSecsSinceEpoch() - dc.executionStart.toSecsSinceEpoch();
    dc.executionEnd = QDateTime::currentDateTime();
    QString hours =  QVariant((dc.executionTime / 3600)).toString();
    QString minuts = QVariant(dc.executionTime % 3600 / 60).toString();
    QString secs = QVariant(dc.executionTime % 60).toString();
    QString msecs = QVariant((QDateTime::currentMSecsSinceEpoch() - dc.executionStart.toMSecsSinceEpoch())%1000).toString();

    while(hours.size() <2)
        hours = QString("0") + hours;
    while(minuts.size() <2)
        minuts = QString("0") + minuts;
    while(secs.size() <2)
        secs = QString("0") + secs;
    while(msecs.size() <3)
        msecs = QString("0") + msecs;
    if(queryExecutionState == 0)
        msg += " running subqueries: ";
    else if(queryExecutionState == 1)
        msg += " creating sql query: ";
    else if(queryExecutionState == 2)
        msg += " executing sql query: ";
    else if(queryExecutionState == 3)
        msg += " query success, downloading result: ";
    msg += hours;
    msg += ":";
    msg += minuts;
    msg += ":";
    msg += secs;
    msg += ".";
    msg += msecs;

    msg += "   Successfull queries: ";
    msg += QVariant(dc.savefilecount).toString();
    ui->miscStatusLabel->setText(msg);
}
void LoaderWidnow::onQueryBeginCreating()
{
    qDebug()<<"onQueryBeginCreating()";
    ui->miscStatusLabel->setText(QString("creating sql query...start: ") + dc.executionStart.toString());
    queryExecutionState = 1;
    ui->pushButton_3->hide();
    //ui->stopLoadingQueryButton->hide();
}
void LoaderWidnow::onQueryBegin()
{
    qDebug()<<"onQueryBegin()";
    ui->miscStatusLabel->setText(QString("executing sql query...start: ") + dc.executionStart.toString());
    queryExecutionState = 2;
    ui->pushButton_3->hide();
    //ui->stopLoadingQueryButton->hide();
}
void LoaderWidnow::onQuerySuccess()
{
    qDebug()<<"onQuerySuccess()";
    ui->miscStatusLabel->setText(QString("query success, downloading...start: ") + dc.executionStart.toString());
    queryExecutionState = 3;
    ui->stopLoadingQueryButton->show();
    ui->pushButton_3->hide();
}
void LoaderWidnow::UpdateTable()
{
    // clear tableData, fill tableView with new data up to  25k rows, update info label

    queryExecutionState = 4;
    executionTimer.stop();

    if(dc.lastLaunchIsError)
    {
        qDebug() << "errpos = " << dc._code_start_pos + dc.lastErrorPos;
        QTextCursor tc = cd->textCursor();
        tc.setPosition(dc._code_start_pos + dc.lastErrorPos);
        tc.select(QTextCursor::WordUnderCursor);
        cd->setTextCursor(tc);
    }


    dc.executionEnd = QDateTime::currentDateTime();
    dc.executionTime = dc.executionEnd.toSecsSinceEpoch() - dc.executionStart.toSecsSinceEpoch();
    ui->stopLoadingQueryButton->hide();
    ui->pushButton_3->show();
    dc.tableDataMutex.lock();
    ui->miscStatusLabel->setText("updating table data...");
    ui->tableWidget->clear();
    ui->tableWidget->setColumnCount(dc.data.headers.size());
    int tabl_size = 25000;
    if(dc.data.tbldata.size()>0)
        tabl_size  = dc.data.tbldata[0].size();

    if(tabl_size  > 25000)
        ui->tableWidget->setRowCount(25000);
    else
        ui->tableWidget->setRowCount(tabl_size );

    ui->tableWidget->setHorizontalHeaderLabels(dc.data.headers);
    for(int i=0;i<dc.data.tbldata.size();i++)
    {
        for (int a=0; (a<dc.data.tbldata[i].size() && a < 25000);a++)
        {
            QTableWidgetItem *item = new QTableWidgetItem(dc.data.tbldata[i][a].toString(),dc.data.tbldata[i][a].typeId());
            ui->tableWidget->setItem(a, i, item);
        }
    }
    QString msg = "";
    msg += QVariant(dc.data.tbldata.size()).toString();
    msg += " : ";
    if(dc.data.tbldata.size()>0)
        msg += QVariant(dc.data.tbldata[0].size()).toString();
    else
        msg += "0";

    QString hours =  QVariant((dc.executionTime / 3600)).toString();
    QString minuts = QVariant(dc.executionTime % 3600 / 60).toString();
    QString secs = QVariant(dc.executionTime % 60).toString();
    QString msecs = QVariant((dc.executionEnd.toMSecsSinceEpoch() - dc.executionStart.toMSecsSinceEpoch())%1000).toString();

    while(hours.size() <2)
        hours = QString("0") + hours;
    while(minuts.size() <2)
        minuts = QString("0") + minuts;
    while(secs.size() <2)
        secs = QString("0") + secs;
    while(msecs.size() <3)
        msecs = QString("0") + msecs;

    msg += "   execution time: ";
    msg += hours;
    msg += ":";
    msg += minuts;
    msg += ":";
    msg += secs;
    msg += ".";
    msg += msecs;
    msg += "   Successfull queries: ";
    msg += QVariant(dc.savefilecount).toString();
    ui->miscStatusLabel->setText(QString("data downloaded ") + msg);
    ui->dataSizeLabel_2->setText(msg);
    ui->tableDBNameLabel->setText(dc.dbname);
    dc.tableDataMutex.unlock();
    if(autosaveXLSX)
    {
        ui->saveLineEdit->setText(autofilename);

        on_SaveXLSXButton_pressed();
    }
    if(autosaveSQLITE)
    {

        ui->saveLineEdit->setText(autofilename);

        on_SaveSQLiteButton_pressed();
    }

    on_graph_group_change(gw.groupBysb.value());
    on_graph_separator_change(gw.separateBysb.value());
    on_graph_data_change(gw.dataColumnsb.value());
    //UpdateGraph();
    emit TableUpdated();
}

void LoaderWidnow::OpenFile()
{
    qDebug()<<"OpenFile()";
    QString str= documentsDir + "/" +"excel/";
    str += userDS.data["ExcelPrefixAliases"][cd->highlighter->dbSchemaName.toStdString()];
    str+= ui->saveLineEdit->text();
    if(!str.endsWith(".xlsx"))
        str+= ".xlsx";
    qDebug()<<"opening:" << str;
    if(QDesktopServices::openUrl(QUrl::fromLocalFile(str)))
        ui->miscStatusLabel->setText("Opening file.");
    else
        ui->miscStatusLabel->setText("error opening file.");
}
void LoaderWidnow::OpenNewWindow()
{
    qDebug()<<"OpenNewWindow()";
    QString str= appfilename;
    qDebug()<<"opening:" << str;
    QDesktopServices::openUrl(QUrl::fromLocalFile(str));
}

void LoaderWidnow::ShowIterateWindow()
{
    qDebug()<<"ShowIterateWindow() depracated";
    b_showIteratorWindow = !b_showIteratorWindow;
    if(b_showIteratorWindow)
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
void LoaderWidnow::ShowGraph()
{
    if(gw.groupBysb.isVisible())
    {
        gw.graphThemeCheckBox.hide();
        gw.showLabelsCheckBox.hide();
        gw.saveAsPDFButton.hide();
        gw.groupBysb.hide();
        gw.separateBysb.hide();
        gw.dataColumnsb.hide();
        gw.groupByLabel.hide();
        gw.separateByLabel.hide();
        gw.dataColumnLabel.hide();
        gw.buildGraphButton.hide();
        gw.cv.hide();
        gw.ls.hide();
        gw.bs.hide();
        gw.graphTypeCB.hide();
        gw.graphTypeLabel.hide();
    }
    else
    {
        gw.graphThemeCheckBox.show();
        gw.showLabelsCheckBox.show();
        gw.saveAsPDFButton.show();
        gw.groupBysb.show();
        gw.separateBysb.show();
        gw.dataColumnsb.show();
        gw.groupByLabel.show();
        gw.separateByLabel.show();
        gw.dataColumnLabel.show();
        gw.buildGraphButton.show();
        gw.cv.show();
        gw.ls.show();
        gw.bs.show();
        gw.graphTypeCB.show();
        gw.graphTypeLabel.show();

        on_graph_group_change(gw.groupBysb.value());
        on_graph_separator_change(gw.separateBysb.value());
        on_graph_data_change(gw.dataColumnsb.value());

    }
}
void LoaderWidnow::ShowHistoryWindow()
{
    qDebug()<<"ShowHistoryWindow()";
    b_showWorkspaceWindow = false;
    b_showHistoryWindow = !b_showHistoryWindow;
    if(b_showHistoryWindow)
    {
        QDir directory(documentsDir + "/sqlBackup");
        QStringList strl = directory.entryList();
        tmpSql = cd->toPlainText();
        ui->listWidget->clear();
        ui->listWidget->addItem("tmp");
        for(auto x : strl)
        {
            ui->listWidget->addItem(x);
        }
        ui->listWidget->sortItems(Qt::DescendingOrder);
        historyDS.Save("sqlHistoryList.txt");

        ui->listWidget->setFocus();
        ui->listWidget->show();
    }
    if(!b_showHistoryWindow)
    {
        ui->listWidget->hide();
        cd->setFocus();
    }
}
void LoaderWidnow::ShowWorkspacesWindow()
{
    qDebug()<<"ShowWorkspacesWindow()";
    b_showHistoryWindow = false;
    b_showWorkspaceWindow = !b_showWorkspaceWindow;
    if(b_showWorkspaceWindow)
    {
        QDir directory(documentsDir + "/workspaces");
        QStringList strl = directory.entryList();
        SaveWorkspace();

        ui->listWidget->clear();
        for(auto x : strl)
        {
            if(x !="." && x !=".." )
                ui->listWidget->addItem(x);
        }
        ui->listWidget->sortItems(Qt::AscendingOrder);

        //ui->listWidget->setFocus();
        ui->listWidget->show();
    }
    if(!b_showWorkspaceWindow)
    {
        ui->listWidget->hide();
        cd->setFocus();
    }
}

void LoaderWidnow::on_graph_group_change(int val)
{
    if(val>=0 && val <  dc.data.headers.size())
        gw.groupByLabel.setText("group by " +  dc.data.headers[val]);
    else
        gw.groupByLabel.setText("group by nothing!");
}
void LoaderWidnow::on_graph_separator_change(int val)
{
    if(val>=0 && val <  dc.data.headers.size())
        gw.separateByLabel.setText("separate by " +  dc.data.headers[val]);
    else
        gw.separateByLabel.setText("separate by nothing");
}
void LoaderWidnow::on_graph_data_change(int val)
{
    if(val>=0 && val <  dc.data.headers.size())
        gw.dataColumnLabel.setText("data column: " +  dc.data.headers[val]);
    else
        gw.dataColumnLabel.setText("no data!");
}

void LoaderWidnow::UpdateGraph()
{
    // set theme
    if(gw.graphThemeCheckBox.isChecked())
    {
        gw.cv.setBackgroundBrush(QColor::fromRgb(255,255,255));
        gw.chrt.setBackgroundBrush(QColor::fromRgb(255,255,255));
    }
    if(!gw.graphThemeCheckBox.isChecked())
    {
        gw.cv.setBackgroundBrush(QColor::fromRgb(24,24,24));
        gw.chrt.setBackgroundBrush(QColor::fromRgb(24,24,24));
    }
    int groupColumn = gw.groupBysb.value();
    int dataColumn = gw.dataColumnsb.value();
    int separateColumn = gw.separateBysb.value();

    bool separate = true;
    if(separateColumn <= -1 || dataColumn == separateColumn || groupColumn == separateColumn || separateColumn >= dc.data.tbldata.size())
        separate = false;

    if(dataColumn  >= dc.data.tbldata.size())
        return;
    if(groupColumn  >= dc.data.tbldata.size())
        return;

    std::map<QString,std::map<QString,float>> ColumnData; // ColumnData[separator][grouper] == value

    long int maxi = -1000000;
    long int mini = QDateTime::currentSecsSinceEpoch();
    QDateTime maxdt = QDateTime::currentDateTime();
    QDateTime mindt = QDateTime::currentDateTime();
    double maxf = -100000000.0;
    double minf = 100000000.0;
    bool bottomAxisIsDate = false;
    QStringList names;

    if(dc.data.tbldata.size() <=0)
        return;
    if(dc.data.tbldata[groupColumn].size() <1)
        return;

    // use dateAxis or valueAxis
    if(dc.data.tbldata[groupColumn].size() >=2)
    {
        bottomAxisIsDate = dc.data.tbldata[groupColumn][0].toDateTime().isValid() && !dc.data.tbldata[groupColumn][0].toDateTime().isNull();
    }
    else
        bottomAxisIsDate = false;


    // fill ColumnData
    for(int i=0;i < dc.data.tbldata[groupColumn].size();i++)
    {
        bool isReal = false;
        float a = dc.data.tbldata[dataColumn][i].toReal(&isReal);
        if(!isReal)
            a=1; // count
        if(separate)
            ColumnData[dc.data.tbldata[separateColumn][i].toString()][dc.data.tbldata[groupColumn][i].toString()] += a;
        else
            ColumnData["Value1"][dc.data.tbldata[groupColumn][i].toString()] += a;
    }


    // get min and max
    for(auto a : ColumnData) // separation
    {
        for(auto i : a.second) // group
        {
            if(dc.data.tbldata.size() > 1)
            {

                if(i.second > maxf)
                    maxf = i.second;
                if(i.second < minf)
                    minf = i.second;

                if(!bottomAxisIsDate)
                {
                    if(mini > i.first.toInt())
                        mini = i.first.toInt();
                    if(maxi < i.first.toInt())
                        maxi = i.first.toInt();
                }
                else
                {
                    if(QVariant(i.first).toDateTime().toSecsSinceEpoch() > maxi)
                    {
                        maxi = QVariant(i.first).toDateTime().toSecsSinceEpoch();
                        maxdt = QVariant(i.first).toDateTime();
                    }
                    if(QVariant(i.first).toDateTime().toSecsSinceEpoch() < mini)
                    {
                        mini = QVariant(i.first).toDateTime().toSecsSinceEpoch();
                        mindt = QVariant(i.first).toDateTime();
                    }
                }
            }
        }
    }
    /*
    for(int i=0;i<dc.data.tbldata[dataColumn].size();i++)
    {
        if(dc.data.tbldata.size() > 1)
        {

            if(dc.data.tbldata[dataColumn][i].toFloat() > maxf)
                maxf = dc.data.tbldata[dataColumn][i].toFloat();
            if(dc.data.tbldata[dataColumn][i].toFloat() < minf)
                minf = dc.data.tbldata[dataColumn][i].toFloat();

            if(!bottomAxisIsDate)
            {
                if(mini > dc.data.tbldata[groupColumn][i].toInt())
                    mini = dc.data.tbldata[groupColumn][i].toInt();
                if(maxi < dc.data.tbldata[groupColumn][i].toInt())
                    maxi = dc.data.tbldata[groupColumn][i].toInt();
            }
            else
            {
                if(dc.data.tbldata[groupColumn][i].toDateTime().toSecsSinceEpoch() > maxi)
                {
                    maxi = dc.data.tbldata[groupColumn][i].toDateTime().toSecsSinceEpoch();
                    maxdt = dc.data.tbldata[groupColumn][i].toDateTime();
                }
                if(dc.data.tbldata[groupColumn][i].toDateTime().toSecsSinceEpoch() < mini)
                {
                    mini = dc.data.tbldata[groupColumn][i].toDateTime().toSecsSinceEpoch();
                    mindt = dc.data.tbldata[groupColumn][i].toDateTime();
                }
            }
        }

    }
    */



    qDebug()<<mindt ;
    qDebug()<<maxdt ;

    qDebug()<<minf ;
    qDebug()<<maxf ;
    if(minf > 0.0f)
        minf=  0.0f;

    maxf *= 1.0f;

    // graph type setups
    if(gw.graphTypeCB.currentText() == "Spline")
    {
        for(int i=0;i< ColumnData.size();i++)
        {
            QSplineSeries* ls = new QSplineSeries();
            gw.line_series.push_back(ls);
        }
    }
    else if (gw.graphTypeCB.currentText() == "Line")
    {
        for(int i=0;i< ColumnData.size();i++)
        {
            QLineSeries* ls = new QLineSeries();
            gw.Straight_line_series.push_back(ls);
        }
    }

    gw.ls.clear();
    //gw.chrt.removeAllSeries(); // crashes the app, doing it by hand
    gw.chrt.removeSeries(&gw.ls);
    gw.vay.setGridLineColor(QColor::fromRgbF(0,0,0,0));
    gw.da.setGridLineColor(QColor::fromRgbF(0,0,0,0));

    for(int i=0;i<gw.line_series.size();i++)
    {
        gw.line_series[i]->clear();
        gw.chrt.removeSeries(gw.line_series[i]);

        //gw.line_series[i]->setPointLabelsVisible();
        //gw.line_series[i]->setPointLabelsColor(gw.line_series[i]->pen().color());
        //qDebug()<<gw.line_series[i]->pen().color();
        //gw.line_series[i]->setPointLabelsFormat("@yPoint");
        //gw.line_series[i]->setPointLabelsClipping(false);
    }

    for(int i=0;i<gw.Straight_line_series.size();i++)
    {
        gw.Straight_line_series[i]->clear();
        gw.chrt.removeSeries(gw.Straight_line_series[i]);

        //gw.Straight_line_series[i]->setPointLabelsVisible();
        //gw.Straight_line_series[i]->setPointLabelsColor(gw.Straight_line_series[i]->pen().color());
        //qDebug()<<gw.Straight_line_series[i]->pen().color();
        //gw.Straight_line_series[i]->setPointLabelsFormat("@yPoint");
        //gw.Straight_line_series[i]->setPointLabelsClipping(false);
    }

    gw.bs.clear();
    gw.chrt.removeSeries(&gw.bs);

    int ls_iter = 0;
    gw.bar_sets.clear();
    // fill series with data
    for(auto x : ColumnData) // each separator
    {
        if(gw.graphTypeCB.currentText() == "Spline" || gw.graphTypeCB.currentText() == "Line")
        {
            QLineSeries* ls = nullptr;
            if(gw.graphTypeCB.currentText() == "Spline")
                ls = gw.line_series[ls_iter];
            if(gw.graphTypeCB.currentText() == "Line")
                ls = gw.Straight_line_series[ls_iter];


            ls->setName(x.first);
            for(auto i : x.second) // each group
            {
                qDebug()<<(QVariant(i.first).toDateTime().toSecsSinceEpoch()-mini)/double(maxi - mini) * 1.0f << i.second;
                if(!bottomAxisIsDate && QVariant(i.first).toInt() > 0)
                    ls->append((QVariant(i.first).toInt() - mini)/float(maxi-mini) * 1.0f,i.second);
                else if(bottomAxisIsDate)
                    ls->append((QVariant(i.first).toDateTime().toSecsSinceEpoch()-mini)/double(maxi - mini) * 1.0f,i.second);
            }
        }
        else if(gw.graphTypeCB.currentText() == "Bar")
        {
            QBarSet* set = new QBarSet(x.first);
            gw.bar_sets.push_back(set);


            set->remove(0,set->count());
            set->setLabel(x.first);
            for(auto i : x.second) // each group
            {
                set->append(i.second);
            }
            gw.bs.append(set);
        }


        ls_iter++;
    }



    gw.vay.setMax(maxf);
    gw.vay.setMin(minf);

    if(gw.graphTypeCB.currentText() == "Spline")
    {
        for(int i=0;i< ColumnData.size();i++)
            gw.chrt.addSeries(gw.line_series[i]);
    }
    else if (gw.graphTypeCB.currentText() == "Line")
    {
        for(int i=0;i< ColumnData.size();i++)
            gw.chrt.addSeries(gw.Straight_line_series[i]);
    }
    else if (gw.graphTypeCB.currentText() == "Bar")
    {
        gw.chrt.addSeries(&gw.bs);
    }

    // setup chart
    if(!bottomAxisIsDate)
    {
        gw.chrt.removeAxis(&gw.da);
        gw.chrt.addAxis(&gw.vax,Qt::Alignment::enum_type::AlignBottom);
        gw.vax.setMax(maxi);
        gw.vax.setMin(0);
    }
    else
    {
        gw.chrt.removeAxis(&gw.vax);
        gw.chrt.addAxis(&gw.da,Qt::Alignment::enum_type::AlignBottom);
        gw.da.setMax(maxdt);
        gw.da.setMin(mindt);
    }

    ls_iter = 0;
    for(int i = 0 ;i < gw.line_series.size();i++) // each separator
    {

        gw.line_series[i]->attachAxis(&gw.vay);
        gw.vay.setMax(maxf);
        gw.vay.setMin(minf);
        if(gw.showLabelsCheckBox.isChecked())
        {
            gw.line_series[i]->setPointLabelsColor(gw.line_series[i]->pen().color());
            qDebug()<<gw.line_series[i]->pen().color();
            gw.line_series[i]->setPointLabelsFormat("@yPoint");
            gw.line_series[i]->setPointLabelsClipping(false);


            gw.line_series[i]->setPointLabelsColor(gw.line_series[i]->pen().color());
            gw.line_series[i]->setPointLabelsFormat("@yPoint");
            gw.line_series[i]->setPointLabelsClipping(false);
            QFont fnt =  gw.line_series[i]->pointLabelsFont();
            fnt.setBold(true);
            gw.line_series[i]->setPointLabelsFont(fnt);
        }
        gw.line_series[i]->setPointsVisible();



        gw.line_series[i]->setPointLabelsVisible(gw.showLabelsCheckBox.isChecked());
    }



    // series formating setup, after they were added to chart
    for(int i = 0 ;i < gw.Straight_line_series.size();i++) // each separator
    {
        gw.Straight_line_series[i]->attachAxis(&gw.vay);
        gw.vay.setMax(maxf*1.05f);
        gw.vay.setMin(minf);
        if(gw.showLabelsCheckBox.isChecked())
        {
            gw.Straight_line_series[i]->setPointLabelsColor(gw.Straight_line_series[i]->pen().color());
            qDebug()<<gw.Straight_line_series[i]->pen().color();
            gw.Straight_line_series[i]->setPointLabelsFormat("@yPoint");
            gw.Straight_line_series[i]->setPointLabelsClipping(false);

            gw.Straight_line_series[i]->setPointLabelsColor(gw.Straight_line_series[i]->pen().color());
            gw.Straight_line_series[i]->setPointLabelsFormat("@yPoint");
            gw.Straight_line_series[i]->setPointLabelsClipping(false);
            QFont fnt =  gw.Straight_line_series[i]->pointLabelsFont();
            fnt.setBold(true);
            gw.Straight_line_series[i]->setPointLabelsFont(fnt);
        }
        gw.Straight_line_series[i]->setPointLabelsVisible(gw.showLabelsCheckBox.isChecked());
    }

    for(int i = 0 ;i < gw.bar_sets.size();i++) // each separator
    {
        gw.bar_sets[i]->setBorderColor(gw.bar_sets[i]->brush().color());
        gw.bar_sets[i]->pen().setWidthF(0);
        gw.bar_sets[i]->setLabelColor(gw.bar_sets[i]->brush().color());

        gw.bar_sets[i]->setBorderColor(gw.bar_sets[i]->pen().color());
        //set->setLabelColor(QColor::fromRgb(0,0,0));
        QFont fnt = gw.bar_sets[i]->labelFont();
        fnt.setBold(true);
        gw.bar_sets[i]->setLabelFont(fnt);


    }

    if(gw.showLabelsCheckBox.isChecked())
    {
        gw.bs.setLabelsPosition(QAbstractBarSeries::LabelsOutsideEnd);
        gw.bs.setLabelsVisible(true);
    }
    else
        gw.bs.setLabelsVisible(false);

    gw.bs.attachAxis(&gw.vay);

    gw.vay.setMax(maxf*1.10f);
    gw.vay.setMin(minf);

}
void LoaderWidnow::saveGraphAsPDF()
{
    QPdfWriter writer(documentsDir + "/out.pdf");
    writer.setCreator("QTGraph");
    writer.setPageSize(QPageSize::A4);
    QPainter painter(&writer);
    gw.cv.show();
    gw.cv.render(&painter);
    painter.end();
}

void LoaderWidnow::CopySelectionFormTable()
{
    qDebug()<<"CopySelectionFormTable()";
    if(ui->tableWidget->selectedItems().size()<=0)
        return;
    QModelIndexList indexes = ui->tableWidget->selectionModel()->selectedIndexes();
    QAbstractItemModel * model = ui->tableWidget->model();
    QModelIndex previous = indexes.first();
    indexes.removeFirst();
    QString selected_text =  model->data(previous).toString();
    bool first = true;
    for(auto current : indexes)
    {
        if (current.row() != previous.row())
        {
            selected_text.append('\n');
        }
        else
        {
            selected_text.append('\t');
        }
        QVariant data = model->data(current);
        QString text = data.toString();
        selected_text.append(text);

        previous = current;
    }
    QApplication::clipboard()->setText(selected_text);
}
void LoaderWidnow::CopySelectionFormTableSql()
{
    qDebug()<<"CopySelectionFormTableSql()";
    if(ui->tableWidget->selectedItems().size()<=0)
        return;
    QModelIndexList indexes = ui->tableWidget->selectionModel()->selectedIndexes();
    QAbstractItemModel * model = ui->tableWidget->model();
    int column_count = model->columnCount();
    QModelIndex previous = indexes.first();
    indexes.removeFirst();
    QString selected_text =  "";

    selected_text.append(" '");
    selected_text.append(model->data(previous).toString());
    selected_text.append("', \n");
    bool first = true;
    qDebug()<<"ctr+shift+c'ed";
    qDebug()<<"column_count " << column_count;

    for(auto current : indexes)
    {
        QVariant data = model->data(current);
        QString text = data.toString();
        selected_text.append(" '");
        selected_text.append(text);
        selected_text.append("', \n");

        previous = current;
    }
    QApplication::clipboard()->setText(selected_text);
    return;
}

void LoaderWidnow::updatesuggestion()
{
    ui->suggestionLabel->setText(cd->lastSuggestedWord);
    ui->textPosLabel->setText(QVariant(cd->textCursor().blockNumber() + 1).toString() + ":" + QVariant(cd->textCursor().positionInBlock() + 1).toString() + "(" + QVariant(cd->textCursor().position() + 1).toString() + ")");
}
void LoaderWidnow::FillSuggest()
{
    qDebug()<<"FillSuggest()";
    cd->FillsuggestName();

}
void LoaderWidnow::replaceTool()
{
    qDebug() <<"replaceTool()";
    replaceDialog rd;

    rd.replaceWhat = _replacePrev_What;
    rd.replaceWith = _replacePrev_With;
    rd.start_line = _replacePrev_from;
    rd.end_line = _replacePrev_to;
    rd.Init();
    if(rd.exec())
    {// do the raplacing
        cd->replace(rd.start_line,rd.end_line,rd.replaceWhat,rd.replaceWith);
        qDebug() <<"replaced";
    }
    _replacePrev_What = rd.replaceWhat;
    _replacePrev_With = rd.replaceWith;
    _replacePrev_from = rd.start_line;
    _replacePrev_to = rd.end_line;
}

void LoaderWidnow::SaveWorkspace()
{
    qDebug()<<"SaveWorkspace()";
    if(LastWorkspaceName.contains(":"))
    {// saving into global space
        std::ofstream stream2 ((LastWorkspaceName).toLocal8Bit());
        QString text = cd->toPlainText();
        stream2.clear();
        QStringList tokens;

        if(text.startsWith("-- {"))
        {
            QString text2 = text;
            text.clear();
            text.reserve(text2.size());
            int newlinecounter = 0;
            for(int i=0;i<text2.size();i++)
            {
                if(newlinecounter < 1)
                {
                    if(text2[i] == '\n')
                        newlinecounter += 1;
                }
                else
                {
                    text.push_back(text2[i]);

                }
            }
        }
        stream2 << "-- {" << ui->driverComboBox->currentText().toStdString() << "} ";
        stream2 << " {" <<   ui->DBNameComboBox->currentText().toStdString() << "}\n";
        stream2 << text.toStdString();
        stream2.close();
    }
    else if(!LastWorkspaceName.endsWith(".sql"))
        LastWorkspaceName+=".sql";
    std::ofstream stream2 ((QString(documentsDir + "/workspaces/") + LastWorkspaceName).toLocal8Bit().toStdString());
    QString text = cd->toPlainText();
    stream2.clear();
    QStringList tokens;

    if(text.startsWith("-- {"))
    {
        QString text2 = text;
        text.clear();
        text.reserve(text2.size());
        int newlinecounter = 0;
        for(int i=0;i<text2.size();i++)
        {
            if(newlinecounter < 1)
            {
                if(text2[i] == '\n')
                    newlinecounter += 1;
            }
            else
            {
                text.push_back(text2[i]);

            }
        }
    }
    stream2 << "-- {" << ui->driverComboBox->currentText().toStdString() << "} ";
    stream2 << " {" <<   ui->DBNameComboBox->currentText().toStdString() << "}\n";
    stream2 << text.toStdString();
    stream2.close();
}

// export
void LoaderWidnow::on_SaveXLSXButton_pressed()
{
    qDebug()<<"on_SaveXLSXButton_pressed()";
    QString str = documentsDir + "/excel/";

    str += userDS.data["ExcelPrefixAliases"][cd->highlighter->dbSchemaName.toStdString()];
    str += ui->saveLineEdit->text();
    if(str.size() > 200)// backup against too long filenames, cuz windows
        str.resize(200);
    if(!str.endsWith(".xlsx"))
        str += ".xlsx";
    dc.data.sqlCode = dc.sqlCode;
    dc.data.allSqlCode = cd->toPlainText();

    if(dc.data.ExportToExcel(str,0,0,0,0,true))
        ui->miscStatusLabel->setText(QString("Saved as XLSX ") + str);
    else
        ui->miscStatusLabel->setText(QString("Failed to save xlsx, file probably opened") + str);

}
void LoaderWidnow::on_SaveCSVButton_pressed()
{
    qDebug()<<"on_SaveCSVButton_pressed()";
    QString str = documentsDir + "/CSV/";
    if(!cd->highlighter->dbSchemaName.contains('.'))
        str += cd->highlighter->dbSchemaName;
    str += ui->saveLineEdit->text();
    if(str.size() > 200)// backup against too long filenames, cuz windows
        str.resize(200);
    if(!str.endsWith(".csv"))
        str += ".csv";
    if(dc.data.ExportToCSV(str,';',true))
        ui->miscStatusLabel->setText(QString("Saved as CSV ") + str);
    else
        ui->miscStatusLabel->setText(QString("failed to save to CSV, file probably opened") + str);

}
void LoaderWidnow::on_SaveSQLiteButton_pressed()
{
    qDebug()<<"on_SaveSQLiteButton_pressed()";
    if(dc.data.ExportToSQLiteTable(ui->saveLineEdit->text()))
        ui->miscStatusLabel->setText(QString("Saved to SQLite table") + ui->saveLineEdit->text());
    else
        ui->miscStatusLabel->setText(QString("Failed to save to SQLite, check colomn names / repetitions") + ui->saveLineEdit->text());
}

void LoaderWidnow::on_ImportFromCSVButton_pressed()
{
    dc.data.ImportFromCSV(QFileDialog::getOpenFileName(this, tr("Select csv file")),';',true);
    UpdateTable();
    ui->tableDBNameLabel->setText("Imported from csv");
}
void LoaderWidnow::on_importFromExcelButton_pressed()
{
    dc.data.ImportFromExcel(QFileDialog::getOpenFileName(),0,0,0,0,true);
    UpdateTable();
    ui->tableDBNameLabel->setText("Imported from Excel");
}

void LoaderWidnow::on_workspaceLineEdit_textChanged(const QString &arg1)
{
    LastWorkspaceName = arg1;
}
void LoaderWidnow::on_listWidget_currentTextChanged(const QString &currentText)
{
    if(b_showWorkspaceWindow && currentText =="")
        return;

    if(currentText == "tmp")
        cd->setPlainText(tmpSql);
    else
    {
        QString filename = currentText;
        if(b_showWorkspaceWindow)
        {
            filename = documentsDir + "/workspaces/" + currentText;
            LastWorkspaceName = currentText;
            ui->workspaceLineEdit->setText(currentText);
        }
        else
        {
            filename = documentsDir +"/sqlBackup/"+filename;
            LastWorkspaceName= "tmp_history";
            ui->workspaceLineEdit->setText("tmp_history");

        }
        QFile file( filename);
        if (file.open(QFile::ReadOnly | QFile::Text))
            cd->setPlainText(file.readAll());
        if(b_showWorkspaceWindow)
        {
            QString text = cd->toPlainText();
            if(text.startsWith("-- {"))
            {
                QStringList tokens;
                int i = 0;
                bool inbrakets = false;
                while(i < text.size())
                {
                    if(text[i] == '{')
                    {
                        tokens.push_back("");
                        inbrakets = true;
                        i++;
                        continue;
                    }
                    if(text[i] == '}')
                    {
                        tokens.back() = tokens.back().trimmed();
                        inbrakets = true;
                        if(tokens.size() == 2)
                            break;
                        i++;
                        continue;
                    }
                    if(inbrakets)
                    {
                        tokens.back().push_back(text[i]);
                    }
                    i++;

                }
                if(tokens.size() >= 2)
                {
                    ui->driverComboBox->setCurrentText(tokens[0].trimmed());
                    ui->DBNameComboBox->setCurrentText(tokens[1].trimmed());


                    dc.driver = tokens[0].trimmed();
                    dc.dbname = tokens[1].trimmed();

                    if(userDS.Load((documentsDir + "/userdata.txt").toStdString()))
                    {

                        dc.usrname = userDS.data[ui->DBNameComboBox->currentText().toStdString()]["name"].c_str();
                        dc.password = userDS.data[ui->DBNameComboBox->currentText().toStdString()]["password"].c_str();

                        ui->userNameLineEdit->setText(dc.usrname.trimmed());
                        ui->passwordLineEdit->setText(dc.password.trimmed());
                        dc.usrname = dc.usrname.trimmed();
                        dc.password = dc.password.trimmed();
                    }
                    else
                    {
                        ui->userNameLineEdit->setText("");
                        ui->passwordLineEdit->setText("");
                        dc.usrname =  "";
                        dc.password = "";

                    }
                }
            }
        }
    }
}
void LoaderWidnow::on_DBNameComboBox_currentTextChanged(const QString &arg1)
{
    if(!userDS.Load((documentsDir + "/userdata.txt").toStdString()))
        return;
    ui->userNameLineEdit->setText(userDS.GetProperty(ui->DBNameComboBox->currentText().toStdString(),"name").c_str());
    ui->passwordLineEdit->setText(userDS.GetProperty(ui->DBNameComboBox->currentText().toStdString(),"password").c_str());

    userDS.Save((documentsDir + "/userdata.txt").toStdString());
}

void LoaderWidnow::on_stopLoadingQueryButton_pressed()
{
    dc.stopRunning();
}
void LoaderWidnow::on_the500LinesCheckBox_checkStateChanged(const Qt::CheckState &arg1)
{
    if(arg1 == Qt::Checked)
    {
        dc.stopAt500Lines = true;
    }
    else
        dc.stopAt500Lines = false;
}

// token processor test
void LoaderWidnow::on_pushButton_2_pressed()
{
    QString text = cd->toPlainText();
    QDir directory("sqlBackup");
    QStringList strl = directory.entryList();


    if(!tokenProcessor.ds.Load((documentsDir + "/" +"FrequencyMaps/test2.txt").toStdString()))
        qDebug() << "failed to load FrequencyMap: " <<  (documentsDir + "/" +"FrequencyMaps/test2.txt").toStdString();
    for(auto x : strl)
    {
        QFile f("sqlBackup/" + x);
        f.open(QFile::OpenModeFlag::ReadOnly);
        text = f.readAll().toStdString().c_str();

        tokenProcessor.processText(text);
        tokenProcessor.addFrequencies();
    }
    tokenProcessor.ds.Save((documentsDir + "/" +"FrequencyMaps/test2.txt").toStdString());
    QFile fl ((documentsDir + "/" +"FrequencyMaps/tokens.txt"));
    fl.open(QFile::OpenModeFlag::WriteOnly);
    int i =0;
    fl.write("tokens\n");
    fl.write("{\n");

    for(auto a : tokenProcessor.uniqueTokens)
    {
        fl.write((" "+QVariant(i).toString() + " " + a + "\n").toLocal8Bit());
        i++;
    }
    fl.write("}\n");
    fl.close();
}
// run query button
void LoaderWidnow::on_pushButton_3_pressed()
{
    runSqlAsync();
}

// qml test button
inline QQmlApplicationEngine* TestqmlEngine = nullptr;
void LoaderWidnow::on_pushButton_pressed()
{

    TestqmlEngine->load("DBLoadScript.qml");
}



 void LoaderWidnow::on_nnTestRun_pressed()
 {
     qDebug() << "nntestrun undefined";
 }
void LoaderWidnow::on_nnTestLearn_pressed()
 {
     qDebug() << "on_nnTestLearn_pressed undefined";
 }
