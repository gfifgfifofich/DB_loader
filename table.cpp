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
#include <QApplication>
#include <QAbstractItemModel>
#include <QItemSelectionModel>
#include <QModelIndexList>


//#include <QtQml/qqmlengine.h>
//#include <QtQuick/qquickview.h>
//
//#include <QtQml/qqmlengine.h>
//#include <QtQuick/qquickview.h>
//#include <qquickitem.h>
/*





save with datetime

Remove postgre toggle, detect by drivername.toLower().contains("postgre") || drivername.toLower().contains("psql");
add dates, datatypes
multiline edit
workspaces

datasources - table {sqliteTableName, sqlCode, updateInterval, lastUpdate}
if used_in_code, and current - lastUpdate > updateInterval
    run sqlCode,
    save as sqliteTableName
dataSources
(
sqliteTableName text primary key,
sqlDriver text,
sqlDBName text,
sqlUserName text,
sqlPassword text,
sqlCode text,
updateInterval int, -- minuts
lastUpdate datetime
)
*/

class TableLauncher : public QObject
{
public:
    Table* tbl = nullptr;
    virtual bool event( QEvent *ev )
    {
        qDebug() << "spawned window?";
        tbl = new Table;
        tbl->show();
        return true;
    }
};

inline DataStorage userDS;
inline DataStorage historyDS;

Table::Table(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Table)
{
    ui->setupUi(this);


    cd = new CodeEditor();
    ui->horizontalLayout_3->addWidget(cd);

    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_R), this, SLOT(on_pushButton_3_clicked()));
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Return), this, SLOT(on_pushButton_3_clicked()));

    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_E), this, SLOT(on_pushButton_2_clicked()));
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_S), this, SLOT(SaveWorkspace()));

    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_O), this, SLOT(OpenFile()));
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Space), this, SLOT(FillSuggest()));
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_N), this, SLOT(OpenNewWindow()));

    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_I), this, SLOT(ShowIterateWindow()));
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_G), this, SLOT(ShowGraph()));
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_C), this, SLOT(CopySelectionFormTable()));


    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_H), this, SLOT(ShowHistoryWindow()));
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_W), this, SLOT(ShowWorkspacesWindow()));



    //QQuickView* g_viewer = new QQuickView;
    //g_viewer->setTitle(QStringLiteral("HelloGraphs"));
    //g_viewer->setSource(QUrl("main.qml"));
    //g_viewer->setResizeMode(QQuickView::SizeRootObjectToView);
    //g_viewer->setColor("black");
    //QQuickItem* obj = g_viewer->rootObject();
    //QObject* chld = obj->findChild<QObject*>("ThelineSeries2");
    //qDebug() << chld->objectName();

    //for(auto i : obj->childItems())
    //    qDebug() << i->objectName();
    //g_viewer->show();


    ax->setMax(100);
    ax->setMin(0);
    ax->applyNiceNumbers();
    ax->setObjectName("asdasdzxczxcz");
    ax->setLabelsColor(Qt::white);
    chrt->addAxis(ax,Qt::Alignment::fromInt(1));
    ax2->setMax(100);
    ax2->setMin(0);
    ax2->applyNiceNumbers();
    ax2->setObjectName("asdasd");
    ax2->setLabelsColor(Qt::white);

    axdt->setMax(QVariant("2026-01-01").toDateTime());
    axdt->setMin(QVariant("2022-01-01").toDateTime());
    axdt->setObjectName("asdasd");
    axdt->setLabelsColor(Qt::white);
    chrt->addAxis(ax2,Qt::Alignment::enum_type::AlignBottom);
    chrt->removeAxis(ax2);
    chrt->addAxis(axdt,Qt::Alignment::enum_type::AlignBottom);
    chrt->removeAxis(ax2);
    ls->append(1,100);
    ls->append(20,50);
    ls->append(30,70);
    ls->append(70,50);
    ls->append(100,1);
    ls->setPointLabelsVisible(true);
    ls->setPointLabelsColor(Qt::white);
    ls->setColor(Qt::red);
    ls->setName("axis 1");
    ls->setUseOpenGL();
    chrt->addSeries(ls);
    cv->backgroundBrush().setColor(Qt::black);
    chrt->setBackgroundBrush(cv->backgroundBrush());
    cv->setChart(chrt);
    cv->hide();
    bottomAxisIsDate = false;

    //QQuickView viewer;



    b_showgraph = true;

    iw.Init();
    ui->horizontalLayout_3->addLayout(&iw.iter_layout);
    ui->horizontalLayout_3->addWidget(cv);





    ui->listWidget->hide();

    //ui->horizontalLayout_3->addWidget(&lsr);

    connect( &iw.button, SIGNAL(pressed()), this, SLOT(IterButtonPressed()));

    connect( cd, SIGNAL(s_suggestedName()), this, SLOT(updatesuggestion()), Qt::QueuedConnection );
    connect( this, SIGNAL(execdReady()), this, SLOT(ThreadReady()), Qt::QueuedConnection );

    if(userDS.Load("userdata.txt"))
    {


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

        if((userDS.GetPropertyAsBool("Flags","Script")))
            ui->CheckBoxScript->setCheckState( Qt::CheckState::Checked);
        else
            ui->CheckBoxScript->setCheckState( Qt::CheckState::Unchecked);




        userDS.Save("userdata.txt");
    }


    //connectDB(conName ,ui->comboBoxDriver->currentText(), ui->comboBox->currentText(),ui->lineEdit_2->text(),ui->lineEdit_3->text());


    if(!LastWorkspaceName.endsWith(".sql"))
        LastWorkspaceName+=".sql";
    QFile file("workspaces/" + LastWorkspaceName);
    if (file.open(QFile::ReadOnly | QFile::Text))
        cd->setPlainText(file.readAll());

    cd->setPlainText(cd->toPlainText());


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
            workspaceDependenices.clear();
            ui->comboBoxDriver->setCurrentText(tokens[0].trimmed());
            ui->comboBox->setCurrentText(tokens[1].trimmed());
            ui->lineEdit_2->setText(tokens[2].trimmed());
            ui->lineEdit_3->setText(tokens[3].trimmed());


            t_driver =   tokens[0].trimmed();
            t_dbname =   tokens[1].trimmed();
            t_usrname =  tokens[2].trimmed();
            t_password = tokens[3].trimmed();
            LastWorkspaceUpdateInterval = QVariant(tokens[4].trimmed()).toULongLong();
            LastWorkspaceUpdate = QVariant(tokens[5].trimmed()).toULongLong();
            workspaceDependenices = tokens[6].trimmed().split(',');


        }
    }

    cd->setFocus();
}
void Table::Init(QString conname,QString driver,QString dbname,QString usrname, QString password, QString sql, QString SaveFileName)
{
    ui->comboBoxDriver->setCurrentText(driver);
    ui->comboBox->setCurrentText(dbname);
    ui->lineEdit_2->setText(usrname);
    ui->lineEdit_3->setText(password);
    ui->lineEdit->setText(SaveFileName);
    cd->setPlainText(sql);
    ui->lineEdit_4->setText(LastWorkspaceName);
    sqlCode = sql;
    ui->CheckBoxScript->setCheckState(Qt::Unchecked);
}

void Table::CopySelectionFormTable()
{
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

void Table::ShowGraph()
{
    if(b_showgraph)
    {
        cv->hide();
        b_showgraph = false;
    }
    else if(!b_showgraph)
    {
        cv->show();
        b_showgraph = true;
        UpdateGraphGraph();
    }
}
void Table::UpdateGraphGraph()
{
    ax->applyNiceNumbers();
    ax->setObjectName("asdasdzxczxcz");
    ax->setLabelsColor(Qt::white);
    ax2->applyNiceNumbers();
    ax2->setObjectName("asdasd");
    ax2->setLabelsColor(Qt::white);
    ls->clear();
    long int maxi = 0;
    long int mini = QDateTime::currentSecsSinceEpoch();
    QDateTime maxdt = QDateTime::currentDateTime();
    QDateTime mindt = QDateTime::currentDateTime();
    float maxf = -100000000.0f;
    float minf = 100000000.0f;
    if(tableData.size() <=0)
        return;
    if(tableData[0].size() <1)
        return;
    if(tableData[0].size() >=2)
    {
        bottomAxisIsDate = tableData[0][0].toDateTime().isValid() && !tableData[0][0].toDateTime().isNull();
    }
    else
        bottomAxisIsDate = false;

    for(int i=0;i<tableData.size();i++)
    {
        if(tableData[i][0].toFloat() > maxf)
            maxf = tableData[i][0].toFloat();
        if(tableData[i][0].toFloat() < minf)
            minf = tableData[i][0].toFloat();
        if(!bottomAxisIsDate)
            maxi = i;
        else
        {
            if(tableData[i][1].toFloat() > maxf)
                maxf = tableData[i][1].toFloat();
            if(tableData[i][1].toFloat() < minf)
                minf = tableData[i][1].toFloat();

            if(tableData[i][0].toDateTime().toSecsSinceEpoch() > maxi)
            {
                maxi = tableData[i][0].toDateTime().toSecsSinceEpoch();
                maxdt = tableData[i][0].toDateTime();
            }
            if(tableData[i][0].toDateTime().toSecsSinceEpoch() < mini)
            {
                mini = tableData[i][0].toDateTime().toSecsSinceEpoch();
                mindt = tableData[i][0].toDateTime();
            }
        }

    }
    if(minf > 0.0f)
        minf=  0.0f;

    maxf *= 1.05f;
    if(!bottomAxisIsDate)
        for(int i=0;i<tableData.size();i++)
        {
            ls->append(i/float(maxi) * 100,(tableData[i][0].toReal()) / (maxf-minf) * 100);
        }
    else
    {
        for(int i=0;i<tableData.size();i++)
        {
            qDebug()<<(tableData[i][0].toDateTime().toSecsSinceEpoch()-mini)/double(maxi - mini) * 100.0f;
            ls->append((tableData[i][0].toDateTime().toSecsSinceEpoch()-mini)/double(maxi - mini) * 100.0f,(tableData[i][1].toReal()) / (maxf-minf) * 100);
        }
    }
    ax->setMax(maxf);

    ax->setMin(minf);
    if(!bottomAxisIsDate)
    {
        chrt->removeAxis(axdt);
        chrt->addAxis(ax2,Qt::Alignment::enum_type::AlignBottom);
        ax2->setMax(maxi);
        ax2->setMin(0);
    }
    else
    {
        chrt->removeAxis(ax2);
        chrt->addAxis(axdt,Qt::Alignment::enum_type::AlignBottom);
        axdt->setMax(maxdt);
        axdt->setMin(mindt);
    }
    ls->setMarkerSize(100);
    ls->setPointLabelsVisible(true);
    ls->setPointLabelsColor(Qt::white);
    ls->setColor(Qt::red);
    ls->setName("axis 1");
    ls->setUseOpenGL();
    cv->backgroundBrush().setColor(Qt::black);
    chrt->setBackgroundBrush(cv->backgroundBrush());
    //cv->setChart(chrt);
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
        tbl->Init(QVariant(thrnum).toString(), ui->comboBoxDriver->currentText(),ui->comboBox->currentText(), ui->lineEdit_2->text(),  ui->lineEdit_3->text(),formatedSql,autofilename);
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
bool viewall = false;
void Table::UpdateTable()
{
    tableDataMutex.lock();
    qDebug()<<"updating table";
    ui->statuslabel->setText("updating table data...");
    ui->tableWidget->clear();
    qDebug()<<"celared table";
    qDebug()<<headers.size();
    ui->tableWidget->setColumnCount(headers.size());
    qDebug()<<"column count";
    int tabl_size = 5000;
    if(tableData.size()>0)
        tabl_size  = tableData[0].size();

    if(viewall)
        ui->tableWidget->setRowCount(tabl_size );
    else if(tabl_size  > 5000)
        ui->tableWidget->setRowCount(5000);
    else
        ui->tableWidget->setRowCount(tabl_size );

    qDebug()<<"rowcount";
    ui->tableWidget->setHorizontalHeaderLabels(headers);
    qDebug()<<"labels";
    for(int i=0;i<tableData.size();i++)
    {
        for (int a=0; (viewall && a<tableData[i].size()) || (a<tableData[i].size() && !viewall && a < 5000);a++)
        {
            QTableWidgetItem *item = new QTableWidgetItem(tableData[i][a].toString(),tableData[i][a].typeId());
            ui->tableWidget->setItem(a, i, item);
        }
    }
    QString msg = "table updated. size:";
    msg += QVariant(tableData.size()).toString();
    msg += " : ";
    if(tableData.size()>0)
        msg += QVariant(tableData[0].size()).toString();
    else
        msg += "0";

    ui->statuslabel->setText(msg);
    tableDataMutex.unlock();
    if(autosaveXLSX)
    {
        ui->lineEdit->setText(autofilename);

        SaveToFile();
    }
    if(b_SaveAsWorkSpace)
    {
        autofilename = LastWorkspaceName;
        if(autofilename.endsWith(".sql"))
            autofilename.resize(autofilename.size()-4);
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
    tableDataMutex.unlock();


    if(!tdb.isOpen())
    {
        bool ok = tdb.open();
        if(ok)
            qDebug() << "db opened";
        else
            qDebug() << "nope: "<< tdb.lastError().text().toStdString();
    }
    QString str = sqlCode;
    qDebug() << "creating query";
    QSqlQuery q(str,tdb);
    qDebug() << "created query";

    q.setForwardOnly(true);
    q.prepare(str);
    headers.clear();


    qDebug() << "EXECUTING query";
    if(q.exec(str))
    {
        qDebug() << "query sucess";
        ui->statuslabel->setText("query sucess, downloading result");
        if(closing)
            return;

        for(int a=0,total = q.record().count(); a<total;a++)
            headers<<q.record().fieldName(a);
        int i=0;

        tableData.clear();
        tableData.resize(headers.size());
        while(q.next())
        {
            if(closing)
                return;
            for(int a=0,total = q.record().count(); a<total;a++)
            {
                tableData[a].push_back(q.value(a));
            }
            i++;
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
        qDebug() << sqlCode;
        tableData.back().push_back(q.lastError().text().toStdString().c_str());
        tableData.back().push_back(q.lastError().databaseText());
        tableData.back().push_back(q.lastError().driverText());
    }
    q.clear();
    q.finish();
    qDebug() << "Execd";
    qDebug() << tableData.size() << "  " << tableData[0].size();
    executing = false;
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
    QXlsx::Document* xlsxR3 = new QXlsx::Document();

    if(tableData.size()<=0)
        return;

    qDebug() << "saving";
    qDebug() << "ui->tableWidget->rowCount()"<<ui->tableWidget->rowCount();
    qDebug() << "ui->tableWidget->columnCount()"<<ui->tableWidget->columnCount();
    int sheetnumber = 1;
    int rowoffset = 0;

    QString str = "";
    str+=ui->lineEdit->text();
    qDebug()<< str;

    for(int i=0;i<ui->tableWidget->columnCount();i++)
    {
        QVariant var = ui->tableWidget->horizontalHeaderItem(i)->text();

        xlsxR3->write(1,i+1,var);
    }
    //int datasaved = 0;

    if(tableData.size() > 0 && tableData[0].size() <=1'000'000)
        for(int i=0;i<tableData.size();i++)
        {
            for(int a=0;a<tableData[i].size();a++)
            {
                int row = a + 2 - rowoffset;
                int column = i + 1;
                xlsxR3->write(row,column,tableData[i][a]);
            }
        }
    else
    {
        bool stop = false;
        int start = 0;
        int end = 1'000'000;
        while(!stop)
        {
            qDebug()<<start << end;
            for(int i=0;i<tableData.size();i++)
            {
                for(int a=start ;a<end ;a++)
                {
                    int row = a + 2 - start;
                    int column = i + 1;
                    xlsxR3->write(row,column,tableData[i][a]);
                }
            }

            if(end >= tableData[0].size())
                break;
            sheetnumber++;

            start = end;
            end += 1'000'000;
            if(end > tableData[0].size())
                end = tableData[0].size();

            QString sheetname = "Sheet";
            sheetname += QVariant(sheetnumber).toString();
            xlsxR3->addSheet(sheetname);
            xlsxR3->selectSheet(sheetname);
            for(int asdi=0;asdi<ui->tableWidget->columnCount();asdi++)
            {
                QVariant var = ui->tableWidget->horizontalHeaderItem(asdi)->text();
                xlsxR3->write(1,asdi+1,var);
            }
        }
    }

    //for(int i=0;i<tableData.size();i++)
    //{
    //    for(int a=0;a<tableData[i].size();a++)
    //    {

    //        int row =i+2 - rowoffset;
    //        int column =a+1;

    //        xlsxR3->write(row,column,tableData[i][a]);

    //    }
    //    if(i+2 - rowoffset > 1'000'000)
    //    {
    //        sheetnumber++;
    //        rowoffset = i + 1;
    //        QString sheetname = "Sheet";
    //        sheetname += QVariant(sheetnumber).toString();
    //        xlsxR3->addSheet(sheetname);
    //        xlsxR3->selectSheet(sheetname);
    //        for(int asdi=0;asdi<ui->tableWidget->columnCount();asdi++)
    //        {
    //            QVariant var = ui->tableWidget->horizontalHeaderItem(asdi)->text();

    //            xlsxR3->write(1,asdi+1,var);
    //        }

    //    }
    //}

    xlsxR3->addSheet("SQL");
    xlsxR3->selectSheet("SQL");
    xlsxR3->write(1,1,"SQL QUERY");
    xlsxR3->write(2,1,sqlCode);
    xlsxR3->write(3,1,"all SQL Code");
    xlsxR3->write(4,1,cd->toPlainText());
    xlsxR3->selectSheet("Sheet1");
    if(xlsxR3->saveAs(str))
        ui->statuslabel->setText("saved.");
    else
        ui->statuslabel->setText("failed to save, file probably opened.");
    delete xlsxR3;

}

void Table::connectDB(QString conname,QString driver, QString dbname,QString usrname, QString password)
{
    tdb.close();
    bool sqlite = false;
    bool oracle = false;
    bool postgre = false;
    bool ODBC = false;
    cd->highlighter->PostgresStyle = false;
    QString dbSchemaName = "NoName";
    if(driver.trimmed() != "LOCAL_SQLITE_DB")
    {
        if(driver =="QOCI")
        {
            tdb = QSqlDatabase::addDatabase("QOCI",conname);
            tdb.setConnectOptions("OCI_ATTR_PREFETCH_ROWS=1000");
            oracle = true;
            //ui->checkBox->setCheckState(Qt::Unchecked);
        }
        else if(driver =="QPSQL")
        {
            tdb = QSqlDatabase::addDatabase("QPSQL",conname);
            postgre = true;
            cd->highlighter->PostgresStyle = true;
            //ui->checkBox->setCheckState(Qt::Checked);
        }
        else
        {
            tdb = QSqlDatabase::addDatabase("QODBC",conname);
            ODBC = true;
           // ui->checkBox->setCheckState(Qt::Unchecked);
        }
    }
    else
    {
       // ui->checkBox->setCheckState(Qt::Unchecked);
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
    if(oracle)
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
        connectString = dbname;

    }
    else if(postgre)
    {
        connectString.append(dbname.trimmed());

        QStringList strl = dbname.trimmed().split('/');
        QString filename;
        strl = dbname.trimmed().split('/');
        QString ip = "";
        int port = 0;
        if(strl.size() > 1)
        {
            dbSchemaName = strl[1];
            strl = strl[0].split(':');
            if(strl.size()>1)
            {
                ip = strl[0].trimmed();
                port = QVariant(strl[1].trimmed()).toInt();
            }
        }
        else if(strl.size() > 0)
        {
            dbSchemaName = strl[0];
        }
        connectString = dbname;
        tdb.setHostName(ip);
        tdb.setDatabaseName(dbSchemaName);
        tdb.setPort(port);
        tdb.setUserName(usrname);
        tdb.setPassword(password);
        qDebug() << ip << port << dbSchemaName;
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
    if(!postgre)
        tdb.setDatabaseName(connectString);
    tdb.setUserName(usrname);
    tdb.setPassword(password);
    qDebug() << connectString;
    }
    cd->highlighter->QSLiteStyle = sqlite;
    bool ok = tdb.open();
    if(ok)
    {
        qDebug() << "db opened";

        ui->comboBoxDriver->setCurrentText(driver);
        ui->comboBox->setCurrentText(dbname);
        ui->lineEdit_2->setText(usrname);
        ui->lineEdit_3->setText(password);

        t_driver = driver;
        t_dbname = dbname;
        t_usrname =  usrname;
        t_password = password;

        if(userDS.Load("userdata.txt"))
        {
            userDS.data["Flags"]["Postgre"] = "1";
            userDS.GetObject(ui->comboBox->currentText().toStdString())["name"] = ui->lineEdit_2->text().toStdString();
            userDS.GetObject(ui->comboBox->currentText().toStdString())["password"] = ui->lineEdit_3->text().toStdString();
            userDS.data["User"]["lastDriver"] = ui->comboBoxDriver->currentText().toStdString();
            userDS.data["User"]["lastDBName"] = ui->comboBox->currentText().toStdString();

            QString LastTmpDriverName =  ui->comboBoxDriver->currentText();
            QString LastTmpDbName = ui->comboBox->currentText();

            userDS.data["User"]["name"] = ui->lineEdit_2->text().toStdString();
            userDS.data["User"]["password"] = ui->lineEdit_3->text().toStdString();


            //if(ui->checkBox->checkState()== Qt::Checked)
            //    userDS.data["Flags"]["Postgre"] = "1";
            //else
            //    userDS.data["Flags"]["Postgre"] = "0";

            if(ui->CheckBoxScript->checkState()== Qt::Checked)
                userDS.data["Flags"]["Script"] = "1";
            else
                userDS.data["Flags"]["Script"] = "0";






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

            //if(ui->checkBox->checkState()== Qt::Checked)
            //    userDS.data["Flags"]["Postgre"] = "1";
            //else
            //    userDS.data["Flags"]["Postgre"] = "0";
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
    if(waiting)
    {
        active_Windows -=1;
        qDebug()<<"active_Windows called: "<< active_Windows;
        QString labelstr = "Waiting for ";
        labelstr += QVariant(active_Windows).toString();
        labelstr += " queries to finish";
        ui->statuslabel->setText(labelstr);
        if(active_Windows <= 0)
        {
            waiting = false;
            qDebug()<<"resuming";
            RunAsScript(waitongpos);
        }
    }
    if(active_Windows<0)
    {
        qDebug()<<"active_Windows count less than 0, something went wrong";
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
    qDebug()<<"Entering loop";
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

            Table* tbl = new Table;

            tbl->sqlCode = lastSQL;
            tbl->cd->setPlainText(lastSQL);

            active_Windows+=1;
            tbl->autofilename = save_text;
            tbl->autosaveXLSX = in_XLSXSave;
            tbl->autosaveSQLITE = in_SQLITESave;
            tbl->show();
            thrnum++;
            tbl->conName = QVariant(thrnum).toString();
            //tl.tbl->connectDB(QVariant(thrnum).toString(), variables[0].trimmed(),variables[1].trimmed(), variables[2].trimmed(),  variables[3].trimmed());

            tbl->Init(QVariant(thrnum).toString(), variables[0].trimmed(),variables[1].trimmed(), variables[2].trimmed(),  variables[3].trimmed(),lastSQL,save_text);

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
                int maxcount = 100;
                while(lastSQL.back() !=';' && maxcount > 0)
                {
                    lastSQL.resize(lastSQL.size()-1);
                    maxcount--;
                }
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
                //if(tokens.back().trimmed() == "Wait" && active_Windows > 0)
                //{
                //    waiting = true;
                //    waitongpos = i;
                //    qDebug()<<"Waiting for "<<active_Windows << " queries to finish";
                //    QString labelstr = "Waiting for ";
                //    labelstr += QVariant(active_Windows).toString();
                //    labelstr += " queries to finish";
                //    ui->statuslabel->setText(labelstr);
                //    return;
                //}
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
    if(userDS.Load("userdata.txt"))
    {
        userDS.data["Flags"]["Script"] = "1";
        userDS.Save("userdata.txt");
    }
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
    active_Windows =0;
    waiting = false;
    waitongpos = 0;
    ui->statuslabel->setText("Script finished");
}

void  Table::runSqlAsync(QString conname,QString driver,QString dbname,QString usrname, QString password, bool createconnection, bool runall)
{

    if(userDS.Load("userdata.txt"))
    {
        userDS.GetObject("User")["lastDriver"] = ui->comboBoxDriver->currentText().toStdString();
        userDS.GetObject("User")["lastDBName"] = ui->comboBox->currentText().toStdString();
        userDS.GetObject("User")["name"] = ui->lineEdit_2->text().toStdString();
        userDS.GetObject("User")["password"] = ui->lineEdit_3->text().toStdString();

        userDS.GetObject(ui->comboBox->currentText().toStdString())["name"] = ui->lineEdit_2->text().toStdString();
        userDS.GetObject(ui->comboBox->currentText().toStdString())["password"] = ui->lineEdit_3->text().toStdString();

        if(cd->highlighter->PostgresStyle)
            userDS.data["Flags"]["Postgre"] = "1";
        else
            userDS.data["Flags"]["Postgre"] = "0";
        if(ui->CheckBoxScript->checkState()== Qt::Checked)
            userDS.data["Flags"]["Script"] = "1";
        else
            userDS.data["Flags"]["Script"] = "0";

        userDS.Save("userdata.txt");
    }
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
            if(start<0)
                start = 0;
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
            sqlCode = tr(cd->textCursor().selectedText().toLocal8Bit());

        if(runall)
        {
            sqlCode = cd->toPlainText();
        }

        if(!ui->ignoreDepsCheckBox->isChecked())
        {
            qDebug()<<"Processing used datasources";
            std::vector<QStringList> filestorun;
            qDebug()<<workspaceDependenices;
            for(int i=0;i< workspaceDependenices.size();i++)
            {
                QString wsname = workspaceDependenices[i];
                qDebug()<<wsname ;
                QString filename = "workspaces/" + workspaceDependenices[i];
                if(!filename.endsWith(".sql"))
                    filename+=".sql";
                QString text = "";
                QFile file(filename);
                if (file.open(QFile::ReadOnly | QFile::Text))
                    text = file.readAll();

                if(text.startsWith("-- {"))
                {
                    QStringList tokens;
                    QString sql = "";
                    int i = 0;
                    bool inbrakets = false;
                    bool gotnewline = false;
                    bool stoptokens = false;
                    while(i < text.size())
                    {
                        //if(!gotnewline && text[i] == '\n')
                        //{
                        //    gotnewline = true;
                        //    i++;
                        //    continue;
                        //}
                        sql+=text[i];
                        if(!stoptokens)
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
                                inbrakets = false;
                                if(tokens.size() == 7)
                                    stoptokens = true;
                                i++;
                                continue;
                            }
                            if(inbrakets)
                            {
                                tokens.back().push_back(text[i]);
                            }
                        }
                        if(stoptokens && text[i] == ';' || i + 1 == text.size())
                        {
                            tokens.push_back(sql);
                            qDebug()<<"sql is " << tokens.size()-1 << " code " << sql;
                            break;
                        }
                        //if(gotnewline)
                        i++;

                    }
                    qDebug()<<"tokens.size()"<<tokens.size();
                    if(tokens.size() >= 7)
                    {

                        qint64 dsWorkspaceUpdateInterval = QVariant(tokens[4].trimmed()).toULongLong();
                        qint64 dsWorkspaceUpdate = QVariant(tokens[5].trimmed()).toULongLong();
                        qDebug()<<QDateTime::currentDateTime().toSecsSinceEpoch() - dsWorkspaceUpdate;
                        qDebug()<< "dsWorkspaceUpdateInterval "<<dsWorkspaceUpdateInterval;
                        if(QDateTime::currentDateTime().toSecsSinceEpoch() - dsWorkspaceUpdate >= dsWorkspaceUpdateInterval)
                        {
                            qDebug()<<"tokens.size()"<<tokens.size();
                            tokens.push_back(wsname);
                            qDebug()<<"tokens.size()"<<tokens.size();
                            filestorun.push_back(tokens);
                            qDebug()<<"filestorun.size()"<<filestorun.size();

                        }
                    }
                }
            }
            qDebug()<<filestorun.size();
            for(int i=0;i<filestorun.size();i++)
            {
                //qDebug()<<filestorun[i];
                t_driver =   filestorun[i][0].trimmed();
                t_dbname =   filestorun[i][1].trimmed();
                t_usrname =  filestorun[i][2].trimmed();
                t_password = filestorun[i][3].trimmed();

                QString str = cd->toPlainText();
                Table* tbl = new Table();
                tbl->sqlCode = filestorun[i][7];
                tbl->cd->setPlainText(filestorun[i][7]);


                tbl->LastWorkspaceName = filestorun[i].back();

                //tbl->autofilename = filestorun[i].back();
                //if(tbl->autofilename.endsWith(".sql"))
                //    tbl->autofilename.resize(tbl->autofilename.size()-4);

                tbl->autosaveSQLITE = true;
                tbl->show();
                tbl->b_SaveAsWorkSpace = true;
                thrnum++;
                tbl->workspaceDependenices.clear();
                tbl->conName = QVariant(thrnum).toString();
                tbl->LastWorkspaceUpdateInterval = QVariant(filestorun[i][4].trimmed()).toULongLong();
                tbl->Init(QVariant(thrnum).toString(), t_driver,t_dbname, t_usrname,  t_password ,filestorun[i][7],filestorun[i].back());
                tbl->workspaceDependenices.clear(); // ignore deps in other ds
                tbl->show();
            }

            if(filestorun.size()>0)
            {
                executing = false;
                ui->statuslabel->setText("Dependencies not satisfied");
                return;
            }
        }

        qDebug()<<"Executing sql:";
        qDebug()<<sqlCode;
        qDebug()<<"";
        while(sqlCode.endsWith(';'))
            sqlCode.resize(sqlCode.size()-1);

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
        if(thr!=nullptr)
            thr->terminate();
        reconect_on_exec = true;
        if(createconnection)
        {
            t_conname = conname;
            t_driver = driver;
            t_dbname = dbname;
            t_usrname = usrname;
            t_password = password;
            reconect_on_exec = true;
            if(reconect_on_exec)
            {
                qDebug() << "creating connection";
                connectDB(t_conname,t_driver,t_dbname,t_usrname,t_password);
                qDebug() << "created connection";
                reconect_on_exec = false;
            }
        }

        thr = QThread::create(Func,this);
        thr->start();
    }
    else
    {
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

        if(historyDS.Load("sqlHistoryList.txt"))
        {
            historyDS.data["wSQL_BACKUP_LIST"][str.toStdString()] = str.toStdString();
            historyDS.Save("sqlHistoryList.txt");
        }


        SaveWorkspace();
        active_Windows =0;
        waiting = false;
        waitongpos = 0;
        RunAsScript();
    }
}

void Table::on_pushButton_3_clicked()
{
    runSqlAsync(conName ,ui->comboBoxDriver->currentText(), ui->comboBox->currentText(),ui->lineEdit_2->text(),ui->lineEdit_3->text(), true);

}

void Table::on_comboBox_currentTextChanged(const QString &arg1)
{
    if(!userDS.Load("userdata.txt"))
        return;
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


    if(tableData.size()>0 && tableData[0].size()>0)
    {
        bool first = true;
        for(int a=0;a<tableData.size();a++)
        {
            if(!first)
                SQLITE_sql += ",";
            first = false;
            SQLITE_sql += " '";
            SQLITE_sql += tableData[a][0].toString();
            SQLITE_sql += "' ";
            SQLITE_sql += " as ";
            SQLITE_sql += " '";
            SQLITE_sql += headers[a];
            SQLITE_sql += "' ";
        }
        //SQLITE_sql += '\n';
    }
    else
    {
        return;
    }
    qDebug() << SQLITE_sql;
    int lasti=0;
    for(int i=1;i<tableData[0].size();i++)
    {

        SQLITE_sql += " union all Select ";
        bool first = true;
        for(int a=0;a<tableData.size();a++)
        {
            if(!first)
                SQLITE_sql += ",";
            first = false;
            int row =i+2;
            int column =a+1;
            bool is_text = false;
            tableData[a][i].toDouble(&is_text);
            is_text = !is_text;

            //if(is_text)

            SQLITE_sql += " '";
            SQLITE_sql += tableData[a][i].toString();
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


            if(tableData[0].size()>lasti)
            {
                bool first = true;
                for(int a=0;a<tableData.size();a++)
                {
                    if(!first)
                        SQLITE_sql += ",";
                    first = false;
                    SQLITE_sql += " '";
                    SQLITE_sql += tableData[a][lasti].toString();
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

void Table::on_listWidget_currentTextChanged(const QString &currentText)
{
    if(currentText == "tmp")
        cd->setPlainText(tmpSql);
    else
    {
        QString filename = currentText;
        if(b_showWorkspaces)
        {
            filename = "workspaces/" + currentText;
            LastWorkspaceName = currentText;
            ui->lineEdit_4->setText(currentText);
        }
        QFile file(filename);
        if (file.open(QFile::ReadOnly | QFile::Text))
            cd->setPlainText(file.readAll());
        if(b_showWorkspaces)
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
                if(tokens.size() == 7)
                {
                    workspaceDependenices.clear();
                    ui->comboBoxDriver->setCurrentText(tokens[0].trimmed());
                    ui->comboBox->setCurrentText(tokens[1].trimmed());
                    ui->lineEdit_2->setText(tokens[2].trimmed());
                    ui->lineEdit_3->setText(tokens[3].trimmed());


                    t_driver =   tokens[0].trimmed();
                    t_dbname =   tokens[1].trimmed();
                    t_usrname =  tokens[2].trimmed();
                    t_password = tokens[3].trimmed();
                    LastWorkspaceUpdateInterval =10;// QVariant(tokens[4].trimmed()).toULongLong();
                    LastWorkspaceUpdate = QVariant(tokens[5].trimmed()).toULongLong();
                    workspaceDependenices = tokens[6].trimmed().split(',');


                }
            }
        }
    }
}
void Table::ShowHistoryWindow()
{
    b_showWorkspaces = false;
    b_showhistory = !b_showhistory;
    if(b_showhistory)
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
    if(!b_showhistory)
    {
        ui->listWidget->hide();
        cd->setFocus();
    }
}

void Table::SaveWorkspace()
{
    if(!LastWorkspaceName.endsWith(".sql"))
        LastWorkspaceName+=".sql";
    std::ofstream stream2 ((QString("workspaces/") + LastWorkspaceName).toLocal8Bit().toStdString());
    QString text = cd->toPlainText();
    stream2.clear();
    QStringList tokens;
    if(text.startsWith("-- {"))
    {

        QString sql = "";
        int i = 0;
        bool inbrakets = false;
        bool gotnewline = false;
        bool stoptokens = false;
        while(i < text.size() && !stoptokens)
        {
            if(!stoptokens)
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
                    inbrakets = false;
                    if(tokens.size() == 7)
                        stoptokens = true;
                    i++;
                    continue;
                }
                if(inbrakets)
                {
                    tokens.back().push_back(text[i]);
                }
            }
            i++;

        }
        qDebug()<<"tokens.size()"<<tokens.size();
        if(tokens.size() >= 7)
        {
            workspaceDependenices = tokens.back().trimmed().split(',');

        }
        if(tokens.size() > 4)
            LastWorkspaceUpdateInterval = QVariant(tokens[4]).toULongLong();

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

    stream2 << "-- {" << ui->comboBoxDriver->currentText().toStdString() << "} ";
    stream2 << " {" <<   ui->comboBox->currentText().toStdString() << "} ";
    stream2 << " {" <<   ui->lineEdit_2->text().toStdString() << "} ";
    stream2 << " {" <<   ui->lineEdit_3->text().toStdString() << "}";
    stream2 << " {" << std::to_string(LastWorkspaceUpdateInterval) << "}";
    stream2 << " {" << QDateTime::currentSecsSinceEpoch() << "}";
    stream2 << " {";
    for(int i=0;i < workspaceDependenices.size();i++)
    {
        stream2 << workspaceDependenices[i].toStdString();
        if(i+1 < workspaceDependenices.size())
            stream2 << ",";
    }
    stream2 <<     "}\n";
    stream2 << text.toStdString();
    stream2.close();

}

void Table::ShowWorkspacesWindow()
{
    b_showhistory = false;
    b_showWorkspaces = !b_showWorkspaces;
    if(b_showWorkspaces)
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
    if(!b_showWorkspaces)
    {
        ui->listWidget->hide();
        cd->setFocus();
    }
}

void Table::on_lineEdit_4_textChanged(const QString &arg1)
{
    LastWorkspaceName = arg1;
}
