

#include <QApplication>
#include <QCoreApplication>
#include <QtQml/qqmlengine.h>
#include <QtQuick/qquickview.h>
#include "Patterns.h"
#include "loaderwidnow.h"
#include "Table.h"
inline int thrnum;

inline QString appfilename;


int main(int argc, char *argv[])
{
    fillPaterns();
    QApplication app(argc, argv);
    app.setStyle("fusion");
    if(argc > 0)
    {
        appfilename = argv[0];
        qDebug()<<"appname is "<< argv[0];
    }

    //Table w;
    //thrnum++;
    //w.conName = QVariant(thrnum).toString();
    //w.show();

    LoaderWidnow w;
    //thrnum++;
    //w.conName = QVariant(thrnum).toString();
    w.show();


    return app.exec();
}


/*
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlComponent>
#include <QQmlContext>
#include "databaseconnection.h"



int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    qmlRegisterType<DatabaseConnection>("SourceApplication", 1, 0, "DatabaseConnection");

    engine.loadFromModule("untitledQML2", "Main");

    return app.exec();
}
*/


/*
 *
TODO:
    create separate class for handling tableData (save, import)
    create separate class for handling all DataBase stuff (queries), using tableData
    only then, re-create UI as frontend for tableData & queries
    then re-create UI in QML/JavaScript, to create proper QML/JavaScript interface
Optional:
    add more highlighting features
    add separate keywords per database type (Oracle, Postgres SQlLite)

// there is a possibility to kill process in databese to kill session/select query process

SELECT service_name, STATE, LOGON_TIME, MODULE, PROGRAM, TERMINAL,PORT, MACHINE, TYPE, SCHEMANAME, OSUSER, STATUS, USERNAME, SID,SERIAL#
FROM V$SESSION
WHERE AUDSID = Sys_Context('USERENV', 'SESSIONID')
and username = 'username';

ALTER SYSTEM CANCEL SQL 'sid, ser, sqlid'; --// will probably kill only if in the same connection?

Schema
TableData
    std::vector<std::vector<QVariant>> // slow
    std::vector<std::vector<QString>> // test, maybe would be faster
    std::vector<QString> headers
    std::vector<int> types // columns types

    void ImportFromCSV(QString fileName, QChar delimeter, bool firstRowHeader);

    void ImportFromExcel(QString fileName, int x_start,int x_end,int y_start,int y_end, bool firstRowHeader);

    void ImportFromSQLiteTable(QString fileName, QString tableName) // load table from sqlite quicker

    void ExportToCSV(QString fileName, QChar delimeter, bool firstRowHeader);

    void ExportToExcel(QString fileName, int x_start,int x_end,int y_start,int y_end, bool firstRowHeader);

    void ExportToSQLiteTable(QString fileName, QString tableName)


// create this class on a different thread to make fully async db stuff
DataBaseConnection
    QString connectionName

    QSqlDatabase db // probably optional, better to use some system with connectionnames and get_db stuff

    int sid, serial, sql_ID; // data from database about current session, get new for each sql query

    TableData data

    Create(driver,DBName,userName, password) // make guid-style connectionname

    execSql(QString sql)

UI
    Iterate
    RunSqlAsync
    ...


*/
