#include "databaseconnection.h"
#include "datastorage.h"
#include <qsqlerror.h>
#include <qsqlrecord.h>
#include "Patterns.h"


inline DataStorage userDS;

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

/*
    - SubexecToArray{
            {driver} {database}
            select * from stufff
            }
    - SubexecToMagic{
            {driver} {database}
            select * from stufff
            }
    - SubexecToUnionAllTable{
            {driver} {database}
            select * from stufff
            }

select stuff
from things
where  stuff in (SubexecToArray{...});



select stuff
from things
where ('magic', stuff) in (SubexecToMagic{...});



with a as (SubexecToUnionAllTable{...}),
    b as (SubexecToUnionAllTable{...})

select * from a cross join b;

std::vector<int> scriptPausePositions;
std::vector<QString> scriptSqls;
std::vector<QString> scriptDrivers;
std::vector<QString> scriptDBNames;
std::vector<QString> scriptCommands;
std::vector<DatabaseConnections> subscriptConnesctions;

*/

void DatabaseConnection::execSql(QString sql)
{
    stopNow = false;// to be shure we wont reject query right after exec
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


    // Prepeare sql to be run, detect any special keywords
    QString formatedSql;
    QString keywordbuff = "";
    std::vector<int> localKeywordsMatch;
    localKeywordsMatch.resize(subCommandPatterns.size());
    userDS.Load("userdata.txt");

    // iterate through keywords, detect similarity
    qDebug()<<"Entered sql subcomand processing ";

    for(int a=0;a < sql.size();a++)
    {
        bool isAPartOfKeyword = false;
        for(int i=0;i<subCommandPatterns.size();i++)
        {
            if(subCommandPatterns[i][localKeywordsMatch[i]].toLower() == sql[a].toLower())
            {
                isAPartOfKeyword = true;
                localKeywordsMatch[i]++;

                if(localKeywordsMatch[i] == subCommandPatterns[i].size())
                { // detected keyword, implement action
                    qDebug()<<"Detected keyword \"" << subCommandPatterns[i] <<"\"";
                    localKeywordsMatch[i] = 0;
                    scriptCommand = subCommandPatterns[i];
                    int buff_a = a;
                    if(subCommandPatterns[i].startsWith("SubexecTo"))
                    {// its a subexec, crop part from next { to }, exec it in subscriptConnection
                        qDebug()<<"Its SubExec";
                        bool beginSubscriptSQL = false;
                        int bracketValue = 0;
                        int bracketCount = 0;
                        scriptCommand = "";
                        while(buff_a < sql.size())
                        {
                            if(bracketValue <=0 && beginSubscriptSQL)
                            {
                                beginSubscriptSQL = false;
                                a = buff_a;
                                qDebug()<< a;
                                break;
                            }
                            if(sql[buff_a]== '{')
                            {
                                beginSubscriptSQL = true;
                                bracketCount++;
                                if(bracketCount <= 3)
                                    scriptCommand.push_back(' ');
                                else
                                    scriptCommand.push_back('{');

                                bracketValue+=1;
                                buff_a++;
                                continue;
                            }
                            if(sql[buff_a]== '}')
                            {
                                bracketValue-=1;
                                if(bracketCount <= 3)
                                    scriptCommand.push_back(' ');
                                else if(bracketValue >0)
                                    scriptCommand.push_back('}');
                                buff_a++;
                                continue;
                            }


                            if(beginSubscriptSQL)
                                scriptCommand.push_back(sql[buff_a]);


                            buff_a++;
                        }

                        qDebug()<< "";
                        qDebug()<< "Subscript command is:";
                        qDebug()<< scriptCommand;
                        qDebug()<< "";
                        QStringList tokens = scriptCommand.split(' ');
                        QString subscriptDriver = "NoDriver";
                        QString subscriptDBname = "NoDatabase";
                        for(auto s : tokens)
                        {
                            if(s.size() <= 1 || s.contains('\n') || s.contains('-')|| s.contains('\t')) // to short
                                continue;
                            if(subscriptDriver == "NoDriver")
                                subscriptDriver = s.trimmed();
                            else if(subscriptDBname == "NoDatabase")
                                subscriptDBname = s.trimmed();
                            else
                                break;
                        }

                        qDebug()<< "subscriptDriver: " << subscriptDriver;
                        qDebug()<< "subscriptDBname: " << subscriptDBname;
                        subscriptConnesction = new DatabaseConnection();

                        QString username = userDS.GetObject(subscriptDBname.toStdString())["name"].c_str();
                        QString password = userDS.GetObject(subscriptDBname.toStdString())["password"].c_str();
                        qDebug()<< subscriptDriver << subscriptDBname << username.trimmed() << password.trimmed();
                        subscriptConnesction->connectionName = connectionName + QVariant(a).toString();
                        subscriptConnesction->Create(subscriptDriver.trimmed(),subscriptDBname.trimmed(),username.trimmed(),password.trimmed());
                        while(scriptCommand.endsWith(' ') ||scriptCommand.endsWith('\t') ||scriptCommand.endsWith('\n') || scriptCommand.endsWith('\r'))
                            scriptCommand.resize(scriptCommand.size()-1);
                        subscriptConnesction->execSql(scriptCommand.trimmed());

                        if(subCommandPatterns[i] == "SubexecToUnionAllTable")
                        {// exec into union all table
                            // Oracle: select * from dual union all select * from dual
                            // PostgreSQL/SQLite select * union all select *...
                            qDebug() <<"Reached SubexecToUnionAllTable implementation";

                            if(subscriptConnesction != nullptr && subscriptConnesction->data.tbldata.size() > 0)
                            {
                                if(subscriptConnesction->data.tbldata[0].size() > 0)
                                {
                                    formatedSql += "Select ";
                                    for(int t=0;t<subscriptConnesction->data.tbldata.size();t++)
                                    {
                                        formatedSql += "'";
                                        formatedSql += subscriptConnesction->data.tbldata[t][0].toString();
                                        formatedSql += "'";
                                        formatedSql += " as \"";
                                        formatedSql += subscriptConnesction->data.headers[t];
                                        formatedSql += "\"";

                                        if(t + 1 <subscriptConnesction->data.tbldata.size()) // there will be next
                                            formatedSql += ",";
                                    }
                                    if(subscriptConnesction->oracle)
                                        formatedSql += "from dual";
                                    for(int j=0;j<subscriptConnesction->data.tbldata[0].size();j++)
                                    {
                                        formatedSql += " union all Select ";
                                        for(int t=0;t<subscriptConnesction->data.tbldata.size();t++)
                                        {
                                            formatedSql += "'";
                                            formatedSql += subscriptConnesction->data.tbldata[t][j].toString();
                                            formatedSql += "'";

                                            if(t + 1 <subscriptConnesction->data.tbldata.size()) // there will be next
                                                formatedSql += ",";
                                        }
                                        if(subscriptConnesction->oracle)
                                            formatedSql += "from dual";
                                    }
                                }
                                else
                                {
                                    formatedSql += "Select ";
                                    for(int t=0;t<subscriptConnesction->data.tbldata.size();t++)
                                    {
                                        formatedSql += "'";
                                        formatedSql += "Empty_query";
                                        formatedSql += "'";
                                        formatedSql += " as \"";
                                        formatedSql += subscriptConnesction->data.headers[t];
                                        formatedSql += "\"";

                                        if(t + 1 <subscriptConnesction->data.tbldata.size()) // there will be next
                                            formatedSql += ",";
                                    }
                                    if(subscriptConnesction->oracle)
                                        formatedSql += "from dual";
                                }

                            }


                        }
                        if(subCommandPatterns[i] == "SubexecToMagic")
                        {// exec into ('magic', 'element1') , ('magic', 'element2')
                            //Probably oracle specific cuz oracle has 1k limit on 'in' arrays
                            qDebug() <<"Reached SubexecToMagic implementation";

                            if(subscriptConnesction != nullptr && subscriptConnesction->data.tbldata.size() > 0)
                            {
                                for(int j=0;j<subscriptConnesction->data.tbldata[0].size();j++)
                                {
                                    for(int t=0;t<subscriptConnesction->data.tbldata.size();t++)
                                    {
                                        formatedSql += "('magic',";
                                        formatedSql += "'";
                                        formatedSql += subscriptConnesction->data.tbldata[t][j].toString();
                                        formatedSql += "'";
                                        formatedSql += ")";
                                        if(j + 1 <subscriptConnesction->data.tbldata[0].size() || t + 1 <subscriptConnesction->data.tbldata.size()) // there will be next
                                            formatedSql += ",";
                                    }
                                }
                            }



                        }
                        if(subCommandPatterns[i] == "SubexecToArray")
                        {// exec into 'element1','element2','element3'
                            qDebug() <<"Reached SubexecToArray implementation";
                            if(subscriptConnesction != nullptr && subscriptConnesction->data.tbldata.size() > 0)
                            {
                                for(int j=0;j<subscriptConnesction->data.tbldata[0].size();j++)
                                {
                                    for(int t=0;t<subscriptConnesction->data.tbldata.size();t++)
                                    {
                                        formatedSql += "'";
                                        formatedSql += subscriptConnesction->data.tbldata[t][j].toString();
                                        formatedSql += "' ";
                                        if(j + 1 <subscriptConnesction->data.tbldata[0].size() || t + 1 <subscriptConnesction->data.tbldata.size()) // there will be next
                                            formatedSql += ",";
                                    }
                                }
                            }




                        }

                    }


                    keywordbuff = "";
                }
            }
            else
            { // broken symbol line, this isn't a keyword
                localKeywordsMatch[i] = 0;
            }
        }
        if(!isAPartOfKeyword)
        {
            formatedSql += keywordbuff;
            keywordbuff = "";
            formatedSql +=sql[a];
        }
        else
            keywordbuff+= sql[a];
    }

    formatedSql += keywordbuff;
    keywordbuff = "";
    qDebug()<<"Exiting sql subcomand processing........";
    qDebug()<<"Resulting formatedSql is";
    //qDebug()<<formatedSql;

    QString str = formatedSql;
    qDebug() << "creating query";
    emit queryBeginCreating();
    QSqlQuery q(db);
    query = &q;
    qDebug() << "created query";

    q.setForwardOnly(true);
    data.headers.clear();
    executionTime = QDateTime::currentSecsSinceEpoch();
    q.prepare(str);
    qDebug() << "executing query";
    emit queryBeginExecuting();
    if(q.exec() && q.isSelect())
    {
        qDebug() << "query sucess.  isSelect = " << q.isSelect();
        emit querySuccess();
        for(int a=0,total = q.record().count(); a<total;a++)
            data.headers << q.record().fieldName(a);
        int i=0;
        data.tbldata.clear();
        data.tbldata.resize(data.headers.size());
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
                    bool force_double = false;
                    bool isint = false;
                    bool forcetext = false;
                    double doub = var.toDouble(&isdouble);
                    var = str;
                    int integ = var.toInt(&isint);
                    var = str;
                    // passes through values like 12312.123123213123123123
                    // fix implemented, currently testing
                    if(str.count('.') + str.count(',') == 1)
                    {// one coma/dot
                        QStringList strl = str.split(',');
                        if(strl.size()==2)
                        {
                            bool ok1 = false;
                            bool ok2 = false;
                            QVariant(strl[0]).toInt(&ok1);
                            QVariant(strl[1]).toInt(&ok2);
                            if(ok1 && ok2)
                                force_double = true;
                        }
                        strl = str.split('.');
                        if(strl.size()==2)
                        {
                            bool ok1 = false;
                            bool ok2 = false;
                            QVariant(strl[0]).toInt(&ok1);
                            QVariant(strl[1]).toInt(&ok2);
                            if(ok1 && ok2)
                                force_double = true;
                        }
                    }
                    if(force_double )
                    {
                        var = QVariant(var.toDouble());
                    }
                    else
                    {
                        if((!dt.isValid() || dt.isNull()) && isdouble && str.size()>9)
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
                        else
                            var = str;
                    }
                }

                data.tbldata[a].push_back(var);
            }
            i++;
            if(stopAt500Lines && i > 500)
                break;
            if(stopNow)
            {
                stopNow=false;
                break;
            }
        }

    }
    else
    {
        data.headers.clear();
        data.tbldata.clear();
        data.tbldata.emplace_back();
        data.headers.push_back("Error");
        data.headers.push_back("db Error");
        data.headers.push_back("driver Error");
        qDebug() << "Error in sql query:"<< q.lastError().text().toStdString().c_str();
        qDebug() << "Error from database:"<< tr(q.lastError().databaseText().toStdString().c_str());
        qDebug() << "Error from driver:"<< q.lastError().driverText();
        qDebug() << sql;
        data.tbldata.back().push_back(q.lastError().text().toStdString().c_str());
        data.tbldata.back().push_back(q.lastError().databaseText());
        data.tbldata.back().push_back(q.lastError().driverText());

        if(!q.isSelect())
            data.tbldata.back().push_back("and query.isSelect() = false, maybe it's not a select statement");

    }

    q.clear();
    q.finish();
    query = nullptr;

    executionTime = QDateTime::currentSecsSinceEpoch() - executionTime;
    qDebug() << "Execd";
    qDebug() << data.tbldata.size() << "  " << data.tbldata[0].size();
    executing = false;
    emit execedSql();
}

TableData* DatabaseConnection::getData()
{
    return &data;
}
QString DatabaseConnection::replace(QString str,QString what, QString with)
{ // love qml strings
    return str.replace(what,with);
}
