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

    QSqlDatabase db;
    QSqlQuery query;
    QString sqlCode;

    QString LastDBName = "";

    QString connectionName = "";
    QString driver = "";
    QString dbname = "";
    QString usrname =  "";
    QString password = "";

    bool sqlite = false;
    bool oracle = false;
    bool postgre = false;
    bool ODBC = false;

    bool executing = false;

    int sid  = -1, serial  = -1, sql_ID = -1; // probably wont be able to use sqlid // probably cant use it at all

    TableData data;

    QMutex tableDataMutex;
    Q_INVOKABLE void Init();

    Q_INVOKABLE bool Create(QString driver, QString DBName, QString userName, QString password);
    Q_INVOKABLE void execSql(QString sql = "");

    explicit DatabaseConnection(QObject *parent = nullptr);

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
