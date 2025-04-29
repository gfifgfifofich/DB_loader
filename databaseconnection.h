#ifndef DATABASECONNECTION_H
#define DATABASECONNECTION_H

#include <QObject>
#include <QQuickItem>
#include <qsqldatabase.h>
#include <qsqlquery.h>
#include "tabledata.h"
#include <QMutex>


class DatabaseConnection : public QObject
{
    Q_OBJECT
    QML_ELEMENT
public:
    explicit DatabaseConnection(QObject *parent = nullptr);


    QSqlDatabase db;
    QSqlQuery* query = nullptr;
    QString sqlCode;

    // Connection info
    QString connectionName = "";
    QString driver = "";
    QString dbname = "";
    QString usrname =  "";
    QString password = "";

    QString schemaName = "";
    QString ipAddres = "";
    QString port = "";

    QString LastDBName = "";
    int savefilecount = 0;

    //Options on data downloading
    bool stopAt500Lines = false;
    bool stopNow = false;

    // Current connected db driver
    bool sqlite = false;
    bool oracle = false;
    bool postgre = false;
    bool ODBC = false;
    bool customOracle = false;

    //Current Executin query info
    bool dataDownloading = false;
    bool executing = false;
    qint64 executionTime = 0;
    QDateTime executionStart = QDateTime::currentDateTime();
    QDateTime executionEnd = QDateTime::currentDateTime();

    // QUery data output
    TableData data;
    QMutex tableDataMutex;

    // Position of current query in code, used to correct error messages from databases
    int _code_start_line = 0;
    int _code_start_pos = 0;
    int lastErrorPos = 0;
    bool lastLaunchIsError = false;



    Q_INVOKABLE void Init();

    Q_INVOKABLE bool Create(QString driver, QString DBName, QString userName, QString password);
    Q_INVOKABLE bool Create(QString driver, QString DBName); // create dc connection, using userdata
    Q_INVOKABLE bool execSql(QString sql = "");

    // QML functions
    Q_INVOKABLE QString replace(QString str,QString what, QString with);
    Q_INVOKABLE QString getDay();
    Q_INVOKABLE QString getMonth();
    Q_INVOKABLE QString getYear();    
    Q_INVOKABLE TableData* getData();



    // Info for scripts, depricated
    int scriptReturnPosition = 0;
    QString scriptCommand = "";
    DatabaseConnection* subscriptConnesction = nullptr;

    void stopRunning();


    //unused id's of current query/session
    int sid  = -1, serial  = -1, sql_ID = -1; // probably wont be able to use sqlid // probably cant use it at all

signals:
    void ConnectionCreated();
    void queryBeginCreating();
    void queryBeginExecuting();
    void querySuccess();//staring to download
    void execedSql();
};

#endif // DATABASECONNECTION_H

/*
    QString connectionName;

    QSqlDatabase db; // probably optional, better to use some system with connectionnames and get_db stuff

    int sid, serial, sql_ID; // data from database about current session, get new for each sql query

    TableData data

    Create(driver,DBName,userName, password) // make guid-style connectionname

    execSql(QString sql)
*/
