#include "databaseconnection.h"
#include <qsqlerror.h>
#include <qsqlrecord.h>



DatabaseConnection::DatabaseConnection(QObject *parent)
    : QObject{parent}
{
    Init();
}

void DatabaseConnection::Init()
{

}

bool DatabaseConnection::Create(QString driver, QString dbname, QString username, QString password)
{
    db.close();
    sqlite = false;
    oracle = false;
    postgre = false;
    ODBC = false;
    QString dbSchemaName = "NoName";
    if(driver.trimmed() != "LOCAL_SQLITE_DB")
    {
        if(driver =="QOCI")
        {
            db = QSqlDatabase::addDatabase("QOCI",connectionName);
            db.setConnectOptions("OCI_ATTR_PREFETCH_ROWS=200");
            oracle = true;
        }
        else if(driver =="QPSQL")
        {
            db = QSqlDatabase::addDatabase("QPSQL",connectionName);
            postgre = true;
        }
        else
        {
            db = QSqlDatabase::addDatabase("QODBC",connectionName);
            ODBC = true;
        }
    }
    else
    {
        db = QSqlDatabase::addDatabase("QSQLITE",connectionName);
        if(!dbname.endsWith(".db"))
            dbname = "SQLiteDB.db";

        dbSchemaName = dbname.split('.')[0]; // all before first dot
        username = " ";
        password = " ";
        db.setDatabaseName("SQLiteDB.db");
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
            connectString.append(username.trimmed());
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
            db.setHostName(ip);
            db.setDatabaseName(dbSchemaName);
            db.setPort(port);
            db.setUserName(username);
            db.setPassword(password);
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
            connectString.append(username.trimmed());
            connectString.append(";Pwd=" );
            connectString.append(password.trimmed());
            connectString.append(";" );
        }
        if(!postgre)
            db.setDatabaseName(connectString);
        db.setUserName(username);
        db.setPassword(password);
        qDebug() << connectString;
    }

    bool ok = db.open();
    if(ok)
    {
        qDebug() << "db opened";


        this->driver = driver;
        this->dbname = dbname;
        this->usrname =  username;
        this->password = password;

        LastDBName = dbname;
        return true;
    }
    else
        qDebug() << "nope: "<< db.lastError().text().toStdString();
    return false;
}

void DatabaseConnection::execSql(QString sql)
{
    if(sql.size() <=0)
        sql = sqlCode;
    tableDataMutex.lock();
    tableDataMutex.unlock();


    if(!db.isOpen())
    {
        bool ok = db.open();
        if(ok)
            qDebug() << "db opened";
        else
            qDebug() << "nope: "<< db.lastError().text().toStdString();
    }
    QString str = sql;
    qDebug() << "creating query";
    emit queryBeginCreating();
    QSqlQuery q(str,db);
    qDebug() << "created query";

    q.setForwardOnly(true);
    q.prepare(str);
    data.headers.clear();


    qDebug() << "EXECUTING query";
    emit queryBeginExecuting();
    if(q.exec(str))
    {
        qDebug() << "query sucess";
        emit querySuccess();
        for(int a=0,total = q.record().count(); a<total;a++)
            data.headers << q.record().fieldName(a);
        int i=0;

        data.data.clear();
        data.data.resize(data.headers.size());
        while(q.next())
        {
            for(int a=0,total = q.record().count(); a<total;a++)
            {

                QVariant var = q.value(a);

                if(var.typeId() == 10) // additional type checking
                {
                    QString str = var.toString();
                    QDateTime dt = var.toDateTime();
                    var = str;
                    bool isdouble = false;
                    bool isint = false;
                    bool forcetext = false;
                    double doub = var.toDouble(&isdouble);
                    var = str;
                    int integ = var.toInt(&isint);
                    var = str;
                    if((!dt.isValid() || dt.isNull()) && isdouble && QVariant(integ).toString().size()>9)
                    {
                        isdouble = false;
                        forcetext = true;
                    }
                    if(!forcetext)
                    {

                        if(dt.isValid() && !dt.isNull())
                        {// datetime
                            var = QVariant(dt);
                        }
                        else if(isdouble)
                        {
                            var = QVariant(var.toDouble());
                        }
                        else if(isint)
                        {
                            var = QVariant(var.toInt());
                        }
                    }
                }

                data.data[a].push_back(var);
            }
            i++;
        }
    }
    else
    {
        data.headers.clear();
        data.data.clear();
        data.data.emplace_back();
        data.headers.push_back("Error");
        data.headers.push_back("db Error");
        data.headers.push_back("driver Error");
        qDebug() << "Error in sql query:"<< q.lastError().text().toStdString().c_str();
        qDebug() << "Error from database:"<< tr(q.lastError().databaseText().toStdString().c_str());
        qDebug() << "Error from driver:"<< q.lastError().driverText();
        qDebug() << sql;
        data.data.back().push_back(q.lastError().text().toStdString().c_str());
        data.data.back().push_back(q.lastError().databaseText());
        data.data.back().push_back(q.lastError().driverText());
    }
    q.clear();
    q.finish();
    qDebug() << "Execd";
    qDebug() << data.data.size() << "  " << data.data[0].size();
    executing = false;
    emit execedSql();
}


