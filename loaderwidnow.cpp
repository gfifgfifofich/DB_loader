#include "loaderwidnow.h"
#include "ui_loaderwidnow.h"
#include <qclipboard.h>
#include <qdir.h>
#include <qrandom.h>
#include <qshortcut.h>
#include <fstream>
#include <qsqlerror.h>
#include <QDesktopServices>
#include <QFileDialog>
#include "replacedialog.h"

/*
+-                                         add togglable "add db name into file name" // feature added, not togglable
+                                          add ability to stop downloading query at any point of downloading, mb togglable autopause at 500
+                                          save only driver and db name in workspaces, fetch the rest from userdata, its safer
themes from userdata
+                                          Highlight from bracket to bracket with codeEditor HighLight selection (will highlight background from bracket to bracket)
+                                          Replace window - replaces every "string" with other "string", yea
*/

inline DataStorage userDS;
inline DataStorage historyDS;
inline QString appfilename;
inline int thrnum;
inline QTime lastMultiRunPress = QTime::currentTime();

LoaderWidnow::LoaderWidnow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::LoaderWidnow)
{
    ui->setupUi(this);

    cd = new CodeEditor();
    ui->CodeEditorLayout->addWidget(cd);

    ui->stopLoadingQueryButton->hide();

    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_R), this, SLOT(runSqlAsync())); // Run sql async
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Return), this, SLOT(runSqlAsync()));// Run sql async

    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_E), this, SLOT(on_SaveXLSXButton_pressed())); // save to file
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_S), this, SLOT(SaveWorkspace()));

    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_O), this, SLOT(OpenFile()));
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Space), this, SLOT(FillSuggest()));
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_F), this, SLOT(replaceTool()));


    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_N), this, SLOT(OpenNewWindow()));
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_I), this, SLOT(ShowIterateWindow()));
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_G), this, SLOT(ShowGraph()));
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_C), this, SLOT(CopySelectionFormTable()));
    new QShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_C), this, SLOT(CopySelectionFormTableSql()));
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_B), cd, SLOT(CommentSelected())); // comment code

    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_H), this, SLOT(ShowHistoryWindow()));
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_W), this, SLOT(ShowWorkspacesWindow()));

    iw.Init();
    ui->CodeEditorLayout->addLayout(&iw.iter_layout);
    ui->listWidget->hide();

    connect( &iw.button, SIGNAL(pressed()), this, SLOT(IterButtonPressed()));
    connect( cd, SIGNAL(s_suggestedName()), this, SLOT(updatesuggestion()), Qt::QueuedConnection );
    connect( &dc, SIGNAL(execedSql()), this, SLOT(UpdateTable()), Qt::QueuedConnection );
    connect( &dc, SIGNAL(querySuccess()), this, SLOT(onQuerySuccess()), Qt::QueuedConnection );
    connect( &dc, SIGNAL(queryBeginExecuting()), this, SLOT(onQueryBegin()), Qt::QueuedConnection );
    connect( &dc, SIGNAL(queryBeginCreating()), this, SLOT(onQueryBeginCreating()), Qt::QueuedConnection );



    if(userDS.Load("userdata.txt"))
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

    if(!LastWorkspaceName.endsWith(".sql"))
        LastWorkspaceName+=".sql";
    QFile file("workspaces/" + LastWorkspaceName);
    if (file.open(QFile::ReadOnly | QFile::Text))
        cd->setPlainText(file.readAll());

    //cd->setPlainText(cd->toPlainText());
    cd->highlighter->rehighlight();

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
                if(tokens.size() == 7)
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
        if(tokens.size() >= 7)
        {
            ui->driverComboBox->setCurrentText(tokens[0].trimmed());
            ui->DBNameComboBox->setCurrentText(tokens[1].trimmed());
            ui->userNameLineEdit->setText(tokens[2].trimmed());
            ui->passwordLineEdit->setText(tokens[3].trimmed());
        }
    }

    cd->setFocus();
}
void LoaderWidnow::Init(QString conname,QString driver,QString dbname,QString usrname, QString password, QString sql, QString SaveFileName)
{
    ui->driverComboBox->setCurrentText(driver);
    ui->DBNameComboBox->setCurrentText(dbname);
    ui->userNameLineEdit->setText(usrname);
    ui->passwordLineEdit->setText(password);
    ui->saveLineEdit->setText(SaveFileName);
    cd->setPlainText(sql);
    ui->workspaceLineEdit->setText(LastWorkspaceName);
    dc.sqlCode = sql;
}

LoaderWidnow::~LoaderWidnow()
{
    delete ui;
}

void LoaderWidnow::on_ConnectButton_pressed()
{
    qDebug()<<"on_ConnectButton_pressed()";

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

void _AsyncFunc(LoaderWidnow* loader)
{
    loader->dc.execSql();
}
void LoaderWidnow::runSqlAsync()
{
    qDebug()<<"RunSqlAsync()";
    if(userDS.Load("userdata.txt"))
    {
        userDS.GetObject("User")["lastDriver"] = ui->driverComboBox->currentText().toStdString();
        userDS.GetObject("User")["lastDBName"] = ui->DBNameComboBox->currentText().toStdString();
        userDS.GetObject("User")["name"] = ui->userNameLineEdit->text().toStdString();
        userDS.GetObject("User")["password"] = ui->passwordLineEdit->text().toStdString();

        userDS.GetObject(ui->DBNameComboBox->currentText().toStdString())["name"] = ui->userNameLineEdit->text().toStdString();
        userDS.GetObject(ui->DBNameComboBox->currentText().toStdString())["password"] = ui->passwordLineEdit->text().toStdString();

        userDS.Save("userdata.txt");
    }

    if(dc.executing)
        return;
    QRandomGenerator64 gen;
    QString conname = QVariant(gen.generate()).toString();
    QString driver = ui->driverComboBox->currentText();
    QString dbname = ui->DBNameComboBox->currentText();
    QString usrname = ui->userNameLineEdit->text();
    QString password = ui->passwordLineEdit->text();

    dc.executing = true;
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
        if(text[start]==';')
            start++;
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
        qDebug()<<"sql start "<<start;
        qDebug()<<"sql start "<<iter-1;
    }
    else
        dc.sqlCode = tr(cd->textCursor().selectedText().toLocal8Bit());

    if(runall)
    {
        dc.sqlCode = cd->toPlainText();
    }


    qDebug()<<"Executing sql:";
    ui->miscStatusLabel->setText("running sql...");
    qDebug()<<dc.sqlCode;
    qDebug()<<"";
    while(dc.sqlCode.endsWith(';'))
        dc.sqlCode.resize(dc.sqlCode.size()-1);

    QString str = "sqlBackup/";
    QDate dt = QDate::currentDate();
    str += QVariant(dt.year()).toString();
    str += "_";
    str += QVariant(dt.month()).toString();
    str += "_";
    str += QVariant(dt.day() ).toString();
    str += "_";
    str += QTime::currentTime().toString();
    str +=".sql";
    str.replace(":","_");
    qDebug()<<str;
    std::ofstream stream (str.toStdString());
    stream << cd->toPlainText().toStdString();
    stream.close();
    SaveWorkspace();



    if(historyDS.Load("sqlHistoryList.txt"))
    {
        historyDS.data["wSQL_BACKUP_LIST"][str.toStdString()] = str.toStdString();
        historyDS.Save("sqlHistoryList.txt");
    }



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

    QString SIDsql = "SELECT  s.sid, s.serial#, s.sql_id, s.username, s.program FROM v$session s WHERE  s.type != 'BACKGROUND' AND S.USERNAME = 'PLHOLKOVSKIY'";

    ui->miscStatusLabel->setText("running sql...");
    sqlexecThread = QThread::create(_AsyncFunc,this);
    sqlexecThread->start();
    ui->miscStatusLabel->setText("running sql...");

}
void LoaderWidnow::IterButtonPressed()
{
    if( ui->connectionStatusLabel_2->text() == "Not connected")
    {
        ui->miscStatusLabel->setText("Connect to a database before launchng iterations");
        return;
    }
    if( iw.nameline.text().size() <=0)
    {
        ui->miscStatusLabel->setText("No name to replace to iterate");
        return;
    }
    qDebug()<<"IterButtonPressed()";
    int secs = QTime::currentTime().minute() * 60 + QTime::currentTime().second();
    int secswas = lastMultiRunPress.minute() * 60 + lastMultiRunPress.second();

    int start = iw.sb1.value();
    int end = iw.sb2.value();
    int step = iw.sb3.value();

    if(secs - secswas < 5 || step == 0)
    {
        ui->miscStatusLabel->setText("iteration step must be higher than 0");
        return;
    }
    lastMultiRunPress = QTime::currentTime();
    int sqlstart = 0;
    int sqlend = cd->toPlainText().size();

    dc.sqlCode = "";
    QString text = cd->toPlainText();
    int code_start = cd->textCursor().position()-1;
    if(code_start<0)
        code_start = 0;
    while(code_start>0 && text[code_start]!=';')
        code_start--;
    if(text[code_start]==';')
        code_start++;
    int iter = code_start;
    sqlstart = code_start;
    dc.sqlCode.push_back(text[iter]);
    iter++;
    while(iter<text.size())
    {
        dc.sqlCode.push_back(text[iter]);
        sqlend = code_start;
        if((text[iter]==';'))
            break;
        iter++;
    }
    qDebug()<<"sql start "<<code_start;
    qDebug()<<"sql start "<<iter-1;

    for(int it = start; it <=end;it+=step)
    {
        QString formatedSql = "";
        QString text =dc.sqlCode;
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

        QString str = dc.sqlCode;
        LoaderWidnow* tbl = new LoaderWidnow();
        tbl->dc.sqlCode = formatedSql;
        tbl->cd->setPlainText(formatedSql);



        tbl->autofilename = ui->saveLineEdit->text();
        if(tbl->autofilename.endsWith(".xlsx"))
        {
            tbl->autofilename.resize(tbl->autofilename.size()-5);
            tbl->autofilename += "_";
            tbl->autofilename += QVariant(it).toString();
            tbl->autofilename   += ".xlsx";
            tbl->autosaveXLSX   = true;
            tbl->autosaveSQLITE = false;
        }
        else
        {
            tbl->autosaveXLSX = false;
            tbl->autosaveSQLITE = true;
            tbl->autofilename += "_";
            tbl->autofilename += QVariant(it).toString();
        }

        tbl->show();
        thrnum++;
        tbl->dc.connectionName = QVariant(thrnum).toString();
        tbl->Init(QVariant(thrnum).toString(), ui->driverComboBox->currentText(),ui->DBNameComboBox->currentText(), ui->userNameLineEdit->text(),  ui->passwordLineEdit->text(),formatedSql,autofilename);
        tbl->runall = true;
        tbl->createconnection = true;
        tbl->runSqlAsync();
        tbl->show();
    }
}

void LoaderWidnow::onQueryBeginCreating()
{
    qDebug()<<"onQueryBeginCreating()";
    ui->miscStatusLabel->setText("creating sql query...");
    ui->stopLoadingQueryButton->hide();
}
void LoaderWidnow::onQueryBegin()
{
    qDebug()<<"onQueryBegin()";
    ui->miscStatusLabel->setText("executing sql query...");
    ui->stopLoadingQueryButton->hide();
}
void LoaderWidnow::onQuerySuccess()
{
    qDebug()<<"onQuerySuccess()";
    ui->miscStatusLabel->setText("query success, downloading...");
    ui->stopLoadingQueryButton->show();
}
void LoaderWidnow::UpdateTable()
{
    ui->stopLoadingQueryButton->hide();
    dc.tableDataMutex.lock();
    qDebug()<<"updating table";
    ui->miscStatusLabel->setText("updating table data...");
    ui->tableWidget->clear();
    qDebug()<<"celared table";
    qDebug()<<dc.data.headers.size();
    ui->tableWidget->setColumnCount(dc.data.headers.size());
    qDebug()<<"column count";
    int tabl_size = 25000;
    if(dc.data.data.size()>0)
        tabl_size  = dc.data.data[0].size();

    if(tabl_size  > 25000)
        ui->tableWidget->setRowCount(25000);
    else
        ui->tableWidget->setRowCount(tabl_size );

    qDebug()<<"rowcount";
    ui->tableWidget->setHorizontalHeaderLabels(dc.data.headers);
    qDebug()<<"labels";
    for(int i=0;i<dc.data.data.size();i++)
    {
        for (int a=0; (a<dc.data.data[i].size() && a < 25000);a++)
        {
            QTableWidgetItem *item = new QTableWidgetItem(dc.data.data[i][a].toString(),dc.data.data[i][a].typeId());
            ui->tableWidget->setItem(a, i, item);
        }
    }
    QString msg = "";
    msg += QVariant(dc.data.data.size()).toString();
    msg += " : ";
    if(dc.data.data.size()>0)
        msg += QVariant(dc.data.data[0].size()).toString();
    else
        msg += "0";

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

    emit TableUpdated();
}

void LoaderWidnow::OpenFile()
{
    qDebug()<<"OpenFile()";
    QString str= "excel/";
    if(!cd->highlighter->dbSchemaName.contains('.'))
        str += cd->highlighter->dbSchemaName;
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
    qDebug()<<"ShowIterateWindow()";
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
    qDebug()<<"ShowGraph() to be done";

}
void LoaderWidnow::ShowHistoryWindow()
{
    qDebug()<<"ShowHistoryWindow()";
    b_showWorkspaceWindow = false;
    b_showHistoryWindow = !b_showHistoryWindow;
    if(b_showHistoryWindow)
    {
        if(historyDS.Load("sqlHistoryList.txt"))
        {
            std::map sqlbackupmap = historyDS.data["wSQL_BACKUP_LIST"];
            tmpSql = cd->toPlainText();
            ui->listWidget->clear();
            ui->listWidget->addItem("tmp");
            for(auto x : sqlbackupmap)
            {
                ui->listWidget->addItem(x.first.c_str());
            }
            ui->listWidget->sortItems(Qt::DescendingOrder);
            historyDS.Save("sqlHistoryList.txt");
        }
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
        QDir directory("workspaces");
        QStringList strl = directory.entryList();
        SaveWorkspace();

        ui->listWidget->clear();
        for(auto x : strl)
        {
            if(x !="." && x !=".." )
                ui->listWidget->addItem(x);
        }
        ui->listWidget->sortItems(Qt::DescendingOrder);

        //ui->listWidget->setFocus();
        ui->listWidget->show();
    }
    if(!b_showWorkspaceWindow)
    {
        ui->listWidget->hide();
        cd->setFocus();
    }
}

void LoaderWidnow::CopySelectionFormTable()
{
    qDebug()<<"CopySelectionFormTable()";
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
    //qDebug()<<"updatesuggestion()";
    ui->suggestionLabel->setText(cd->lastSuggestedWord);

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
    if(!LastWorkspaceName.endsWith(".sql"))
        LastWorkspaceName+=".sql";
    std::ofstream stream2 ((QString("workspaces/") + LastWorkspaceName).toLocal8Bit().toStdString());
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
    stream2 << " {" <<   ui->DBNameComboBox->currentText().toStdString() << "};\n";
    stream2 << text.toStdString();
    stream2.close();
}
void LoaderWidnow::on_SaveXLSXButton_pressed()
{
    qDebug()<<"on_SaveXLSXButton_pressed()";
    QString str = "excel/";
    if(!cd->highlighter->dbSchemaName.contains('.'))
        str += cd->highlighter->dbSchemaName;
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
    QString str = "CSV/";
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
            filename = "workspaces/" + currentText;
            LastWorkspaceName = currentText;
            ui->workspaceLineEdit->setText(currentText);
        }
        QFile file(filename);
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

                    if(userDS.Load("userdata.txt"))
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
    if(!userDS.Load("userdata.txt"))
        return;
    ui->userNameLineEdit->setText(userDS.GetProperty(ui->DBNameComboBox->currentText().toStdString(),"name").c_str());
    ui->passwordLineEdit->setText(userDS.GetProperty(ui->DBNameComboBox->currentText().toStdString(),"password").c_str());

    userDS.Save("userdata.txt");
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

void LoaderWidnow::on_stopLoadingQueryButton_pressed()
{
    dc.stopNow = true;
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

