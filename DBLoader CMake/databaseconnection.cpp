#include "databaseconnection.h"
#include "datastorage.h"
#include <qapplication.h>
#include <qmessagebox.h>
#include <qsqldriver.h>
#include <qsqlerror.h>
#include <qsqlrecord.h>
#include <qthread.h>
#include "Patterns.h"
#include "sqlSubfunctions.h"

#include "sqlite3.h"

#include "libpq-fe.h"

/*

created query
executing query
onQueryBeginCreating()
onQueryBegin()
17:42:06: The process crashed.

*/

inline QString usrDir;
inline QString documentsDir;

inline DataStorage userDS;

DatabaseConnection::DatabaseConnection(QObject *parent)
    : QObject{parent}
{
    Init();
}
void DatabaseConnection::Init()
{

}
bool DatabaseConnection::DeleteDbConnection()
{
    if(db.isOpen())
        db.close();
    if(sqlite3DBConnectionIsOpen && sqlite3DBConnection != nullptr)
    {
        sqlite3_interrupt((sqlite3*) sqlite3DBConnection);// in case if something was running
        sqlite3_close((sqlite3*) sqlite3DBConnection);

        sqlite3DBConnection = nullptr;
        sqlite3DBConnectionIsOpen = false;
    }


    if(pg_connected && ptr_PGconn != nullptr)
    {
        // just in case
        qDebug() << "sending cancel to postgres";
        PGcancelConn * cncl = PQcancelCreate((PGconn*) ptr_PGconn);
        PQcancelBlocking(cncl);
        PQcancelFinish(cncl);


        PQfinish((PGconn*) ptr_PGconn);
        ptr_PGconn = nullptr;
        pg_connected = false;
    }
    return true;
}
DatabaseConnection::~DatabaseConnection()
{

    db.close();
    if(sqlite3DBConnectionIsOpen && sqlite3DBConnection != nullptr)
    {
        sqlite3_interrupt((sqlite3*) sqlite3DBConnection);// in case if something was running
        sqlite3_close((sqlite3*) sqlite3DBConnection);

        sqlite3DBConnection = nullptr;
        sqlite3DBConnectionIsOpen = false;
    }


    if(pg_connected && ptr_PGconn != nullptr)
    {
        // just in case
        qDebug() << "sending cancel to postgres";
        PGcancelConn * cncl = PQcancelCreate((PGconn*) ptr_PGconn);
        PQcancelBlocking(cncl);
        PQcancelFinish(cncl);


        PQfinish((PGconn*) ptr_PGconn);
        ptr_PGconn = nullptr;
        pg_connected = false;
    }

}

#ifdef Oracle_OCI_Driver

// really shouldnt've created this class, but yea, here it is. used only when connecting, to delete connection if something went wrong
#include "oracledriver.h"


inline OCIServer* lastlastserv;
inline QString lastOracleError = "";


//inline OracleDriver* currentRunningOracleDriver = nullptr;

/*Oracle straight up crashes whole app if you try doing that when there are more than some amount of columns. Storing MetaData untill very end will delay crash to when user will want to close app, so works. But app will crash instead of closing if user used Oracle driver(
yep, imma gonna shart into memory, and not deal with crashes when attempting to clear MetaData.
*/
// thread_local std::vector<std::vector<oracle::occi::MetaData>> mtl;

#endif

bool DatabaseConnection::Create(QString driver, QString dbname, QString username, QString password)
{
    Last_ConnectError = "";
    if(driver.trimmed() == "LOCAL" || dbname.trimmed() == "LOCAL")
    {
        driver = userDS.data["UserTheme"]["db_drv_Save_table_driver"];
        dbname = userDS.data["UserTheme"]["db_drv_Save_table_Connection"];
        dbname = dbname.replace("documentsDir",documentsDir);

        driver = driver.trimmed();
        dbname = dbname.trimmed();

        usrname = userDS.data[dbname]["name"];
        password = userDS.data[dbname]["password"];
        usrname = usrname.trimmed();
        password = password.trimmed();
    }

    // close previous connections, if they are open
    db.close();
    if(sqlite3DBConnectionIsOpen && sqlite3DBConnection != nullptr)
    {
        sqlite3_interrupt((sqlite3*) sqlite3DBConnection);// in case if something was running
        sqlite3_close((sqlite3*) sqlite3DBConnection);

        sqlite3DBConnection = nullptr;
        sqlite3DBConnectionIsOpen = false;
    }


    if(pg_connected && ptr_PGconn != nullptr)
    {
        // just in case
        qDebug() << "sending cancel to postgres";
        PGcancelConn * cncl = PQcancelCreate((PGconn*) ptr_PGconn);
        PQcancelBlocking(cncl);
        PQcancelFinish(cncl);


        PQfinish((PGconn*) ptr_PGconn);
        ptr_PGconn = nullptr;
        pg_connected = false;
    }

    sqlite = false;
    oracle = false;
    postgre = false;
    ODBC = false;
    customOracle = false;
    customSQlite = false;
    customPSQL = false;

    bool QODBC_Excel = false;
    QString dbSchemaName = "NoName";


    // decide driver type
    if(driver.trimmed() == "LOCAL_SQLITE_DB")
    {
        db = QSqlDatabase::addDatabase("QSQLITE",connectionName);
        if(!dbname.endsWith(".db"))
            dbname = documentsDir + "/SQLiteDB.db";

        dbSchemaName = dbname.split('.')[0]; // all before first dot
        username = " ";
        password = " ";
        db.setDatabaseName(dbname);
        sqlite = true;
    }
    else if(driver.trimmed() =="QOCI")
    {
        db = QSqlDatabase::addDatabase("QOCI",connectionName);
        db.setConnectOptions("OCI_ATTR_PREFETCH_ROWS=5000"); // set prefetch count to load data with speed > 2mbit/s
        oracle = true;
    }
    else if(driver.trimmed() =="QPSQL")
    {
        db = QSqlDatabase::addDatabase("QPSQL",connectionName);
        postgre = true;
    }
    else if (driver.trimmed() =="QODBC_Excel")
    {
        db = QSqlDatabase::addDatabase("QODBC",connectionName);
        QODBC_Excel = true;
        ODBC = true;
    }
    else if (driver.trimmed() =="Oracle")
    {
        oracle = true;
        customOracle = true;
    }
    else if (driver.trimmed() =="SQLite")
    {
        sqlite = true;
        customSQlite = true;

        db = QSqlDatabase::addDatabase("QSQLITE",connectionName);
        if(!dbname.endsWith(".db"))
            dbname = documentsDir + "/SQLiteDB.db";

        dbSchemaName = dbname.split('.')[0]; // all before first dot
        username = " ";
        password = " ";
        db.setDatabaseName(dbname);
    }
    else if (driver.trimmed() == "PostgreSQL")
    {
        customPSQL = true;
        postgre = true;
    }
    else
    {
        db = QSqlDatabase::addDatabase("QODBC",connectionName);
        ODBC = true;
    }

    if(!sqlite){
        QString connectString = "Driver={";
        connectString.append(driver.trimmed());
        connectString.append("};");
        if(oracle && !customOracle)
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
            if(!nodebug) qDebug() << ip << port << dbSchemaName;
        }
        else
        {

            QString server;
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

            schemaName = database;
            ipAddres = server;

            if(QODBC_Excel)
            {
                connectString.append("DRIVER={Microsoft Excel Driver (*.xls, *.xlsx, *.xlsm, *.xlsb)};ReadOnly=0;DBQ=" );
                connectString.append(dbname);
                qDebug() << connectString;
            }
            else
                connectString = dbname;
        }
        if(!postgre)
            db.setDatabaseName(connectString);
        db.setUserName(username);
        db.setPassword(password);
        if(!nodebug) qDebug() << connectString;
    }

    // open db
    if (customOracle)
    {
#ifdef Oracle_OCI_Driver

        try
        {
            OracleDriver od;
            QString connection_string ="(DESCRIPTION=(ADDRESS=(PROTOCOL=tcp) (HOST=" + ipAddres.trimmed() + ") (PORT=" + port.trimmed() + "))(CONNECT_DATA=(SERVICE_NAME=" + schemaName.trimmed() + ")))";
            od.Create(username.trimmed().toStdString(),password.trimmed().toStdString(),connection_string.toStdString());

            if(!nodebug) qDebug() << "db opened";


            this->driver = driver;
            this->dbname = dbname;
            this->usrname =  username;
            this->password = password;

            LastDBName = dbname;
            if(!disableSaveToUserDS && userDS.Load((documentsDir + "/userdata.txt")))
            {
                userDS.data[dbname]["lastDriver"] = this->driver;
                 userDS.Save(documentsDir + "/userdata.txt");
            }
            return true;
        }
        catch (oracle::occi::SQLException ex) {
            if(!nodebug) qDebug() << "error creating connection" << ex.getErrorCode() << ex.getMessage();
            Last_ConnectError = QString().fromStdString(ex.getMessage());
            return false;
        }
#else
        qDebug() << "Oracle OCI driver nor built, set Oracle_OCI_Driver, Oracle_OCI_Include_Directory and Oracle_OCI_Lib_Directory";
        return false;
#endif

    }
    else if (customSQlite)
    {
        sqlite = true;
        customSQlite = true;

        sqlite3 *db;

        if(!dbname.endsWith(".db"))
            dbname = documentsDir + "/SQLiteDB.db";

        dbSchemaName = dbname.split('.')[dbname.split('.').size()-2]; // all before first dot
        username = " ";
        password = " ";

        int rc = sqlite3_open(dbname.toStdString().c_str(), &db);
        if (rc) {
            if(!nodebug) qDebug() << "Can't open database: " << sqlite3_errmsg(db) ;
            sqlite3_close(db);
            sqlite3DBConnectionIsOpen = false;
            Last_ConnectError = sqlite3_errmsg(db);
            return false;
        }
        else
        {

            this->driver = driver;
            this->dbname = dbname;
            this->usrname =  username;
            this->password = password;

            sqlite3DBConnection = db;
            sqlite3DBConnectionIsOpen = true;
            if(!nodebug) qDebug() << "sqlite opened";
            if(!disableSaveToUserDS && userDS.Load((documentsDir + "/userdata.txt")))
            {
                userDS.data[dbname]["lastDriver"] = this->driver;
                if(!disableSaveToUserDS) userDS.Save(documentsDir + "/userdata.txt");
            }
            return true;

        }
    }
    else if (customPSQL)
    {

        dbSchemaName = dbname.split('/').back();
        QString host = dbname.split(':').front();
        QString port = dbname.split('/').front().split(':').back();
        QString str = "dbname=" + dbSchemaName + " host=" + host + " port=" + port + " target_session_attrs=read-write" ;
        if(username.trimmed().size()>0)
            str += " user=" + username ;

        if(password.trimmed().size()>0)
            str += " password=" + password;

        if(!nodebug) qDebug() << "connstr: " << str;
        const char* conninfo = str.toUtf8().constData();
        PGconn* conn = PQconnectdb(conninfo);


        if (PQstatus(conn) != CONNECTION_OK)
        {
            QString errmsg = PQerrorMessage(conn);
            if(!nodebug) qDebug() << "custom Postgres Connection failed: " << errmsg;
            PQfinish(conn);

            if(errmsg.contains("session is read-only"))
            {
                str = "dbname=" + dbSchemaName + " host=" + host + " port=" + port + " " ;
                if(username.trimmed().size()>0)
                    str += " user=" + username ;

                if(password.trimmed().size()>0)
                    str += " password=" + password;

                if(!nodebug) qDebug() << "connstr: " << str;
                const char* conninfo = str.toUtf8().constData();
                PGconn* conn = PQconnectdb(conninfo);


                if (PQstatus(conn) != CONNECTION_OK)
                {
                    if(!nodebug) qDebug() << "custom Postgres Connection failed: " << PQerrorMessage(conn);
                    PQfinish(conn);
                    Last_ConnectError = PQerrorMessage(conn);
                    return false;
                }
                else
                {
                    this->driver = driver;
                    this->dbname = dbname;
                    this->usrname = username;
                    this->password = password;

                    ptr_PGconn = conn;
                    pg_connected = true;
                    customPSQL = true;

                    LastDBName = dbname;
                    if(!disableSaveToUserDS && userDS.Load((documentsDir + "/userdata.txt")))
                    {
                        userDS.data[dbname]["lastDriver"] = this->driver;
                        if(!disableSaveToUserDS) userDS.Save(documentsDir + "/userdata.txt");
                    }
                    return true;
                }
            }

            Last_ConnectError = errmsg;
            return false;
        }
        else
        {
            this->driver = driver;
            this->dbname = dbname;
            this->usrname = username;
            this->password = password;

            ptr_PGconn = conn;
            pg_connected = true;
            customPSQL = true;

            LastDBName = dbname;
            if(!disableSaveToUserDS && userDS.Load((documentsDir + "/userdata.txt")))
            {
                userDS.data[dbname]["lastDriver"] = this->driver;
                if(!disableSaveToUserDS) userDS.Save(documentsDir + "/userdata.txt");
            }

            qDebug() << PQisthreadsafe() << " <-thread safety";
            return true;
        }
    }
    else
    {
        bool ok = db.open();
        if(ok)
        {
            if(!nodebug) qDebug() << "db opened";


            this->driver = driver;
            this->dbname = dbname;
            this->usrname =  username;
            this->password = password;

            LastDBName = dbname;
            if(!disableSaveToUserDS && userDS.Load((documentsDir + "/userdata.txt")))
            {
                userDS.data[dbname]["lastDriver"] = this->driver;
                if(!disableSaveToUserDS) userDS.Save(documentsDir + "/userdata.txt");
            }
            return true;
        }
        else
            if(!nodebug) qDebug() << "nope: "<< db.lastError().text().toStdString();
    }


    return false;
}

// create db connection using user data
bool DatabaseConnection::Create(QString driver, QString DBName)
{
    //if local db selected, swap all data for local database, in which the save goes

    if(driver.trimmed() == "LOCAL" || dbname.trimmed() == "LOCAL")
    {
        driver = userDS.data["UserTheme"]["db_drv_Save_table_driver"];
        dbname = userDS.data["UserTheme"]["db_drv_Save_table_Connection"];
        dbname = dbname.replace("documentsDir",documentsDir);

        driver = driver.trimmed();
        dbname = dbname.trimmed();

    }
    if(!disableSaveToUserDS)
    {
        userDS.Load((documentsDir + "/userdata.txt"));
    }
    this->driver = driver;
    this->dbname = DBName;
    this->usrname = userDS.data[DBName]["name"];
    this->password = userDS.data[DBName]["password"];

    bool success = Create(driver.trimmed(),dbname.trimmed(), usrname.trimmed(), password.trimmed());
    return success ;

}


// function to give to thread to process query async
void _dc_AsyncFunc(DatabaseConnection* tmpcon, QString sql)
{

    qDebug() << "Async: running " << tmpcon->dbname<<tmpcon->driver;
    tmpcon->execSql(sql);
    tmpcon->DeleteDbConnection();
    qDebug() << "Async: ran";
}

QString UnrollAllLoops(QString sql)
{
    QString scriptCommand = "";
    QString formatedSql;
    QString keywordbuff = "";
    std::vector<int> localKeywordsMatch;
    localKeywordsMatch.resize(subCommandPatterns.size());

    // iterate through keywords, detect similarity

    bool reset = true;
    int loopcount = -1;


    // loop while keywords can be possible in code (due to nested loops and other syntax)
    while (reset)
    {

        formatedSql = "";
        keywordbuff = "";
        localKeywordsMatch.clear();
        localKeywordsMatch.resize(subCommandPatterns.size());
        loopcount++;
        reset = false;


        for(int a=0;a < sql.size();a++)
        {
            bool isAPartOfKeyword = false;
            for(int i=0;i<subCommandPatterns.size();i++)
            {
                if(localKeywordsMatch[i] < subCommandPatterns[i].size() && subCommandPatterns[i][localKeywordsMatch[i]].toLower() == sql[a].toLower())
                {
                    isAPartOfKeyword = true;
                    localKeywordsMatch[i]++;

                    if(localKeywordsMatch[i] == subCommandPatterns[i].size() && (subCommandPatterns[i].trimmed() == "ForLoop" ||
                                                                                  subCommandPatterns[i].trimmed() == "QueryForLoop" ||
                                                                                  (subCommandPatterns[i].trimmed() ==  "DBLPasteYearMonthOffset") ||
                                                                                  (subCommandPatterns[i].trimmed() ==  "DBLPasteYearDayOffset") ||
                                                                                  (subCommandPatterns[i].trimmed() ==  "DBLPasteDaysInMonth") ||
                                                                                  (subCommandPatterns[i].trimmed() ==  "DBLPasteDaysInMonthByMonth") ||
                                                                                  (subCommandPatterns[i].trimmed() ==  "DBLPasteDaysInMonthDayOffset") ||
                                                                                  (subCommandPatterns[i].trimmed() ==  "DBLPasteDayOfMonth") ||
                                                                                  (subCommandPatterns[i].trimmed() ==  "DBLPasteMonthDayOffset") ||
                                                                                  (subCommandPatterns[i].trimmed() ==  "DBLPasteNumMonthDayOffset") ||
                                                                                  (subCommandPatterns[i].trimmed() ==  "DBLPasteMonth") ||
                                                                                  (subCommandPatterns[i].trimmed() ==  "DBLPasteNumMonth") ||
                                                                                  subCommandPatterns[i].trimmed() == "ExcelSheetList"))
                    { // detected keyword, implement action
                        qDebug() << "possibly a "<<subCommandPatterns[i].trimmed();
                        bool good = false;
                        for(int it = a +1; it < sql.size();it++)
                        {
                            if(sql[it] == '{')
                            {
                                good = true;
                                break;
                            }
                            else if(sql[it] == ' ' || sql[it] == "   ")
                                continue;
                            else break;
                        }
                        if(!good)
                        {
                            localKeywordsMatch[i] = 0;
                            continue; // break out of commands loop, to go to next char
                        }


                        keywordbuff += sql[a];


                        while(localKeywordsMatch[i] < keywordbuff.size())
                        {
                            formatedSql+= keywordbuff[0];
                            keywordbuff.remove(0,1);
                        }

                        localKeywordsMatch[i] = 0;
                        int buff_a = a;
                        bool beginSubscriptSQL = false;
                        int bracketValue = 0;
                        int bracketCount = 0;
                        bool inlineVariables = true;
                        bool justOutOfBrackets = false;
                        bool searchingFirstBrackets = false;
                        QStringList funcVariables;
                        QString lastVar = "";
                        scriptCommand = "";
                        while(buff_a < sql.size())
                        {
                            if(bracketValue <=0 && beginSubscriptSQL)
                            {
                                beginSubscriptSQL = false;
                                a = buff_a;
                                break;
                            }
                            if(justOutOfBrackets && sql[buff_a] != '-'&& sql[buff_a] != '{'&& sql[buff_a] != '}'&& sql[buff_a] != ' '&& sql[buff_a] != '\t'&& sql[buff_a] != '\n'&& sql[buff_a] != '\t'&& sql[buff_a] != '\r')
                                inlineVariables = false;
                            if(inlineVariables)
                            {

                                if(sql[buff_a] == '}' && bracketValue < 3)
                                {
                                    lastVar = lastVar.trimmed();
                                    funcVariables  << lastVar;
                                    lastVar = "";
                                }
                                else if(bracketValue > 1 && sql[buff_a] != '\t'&& sql[buff_a] != '\t'&& sql[buff_a] != '\r')
                                {
                                    lastVar += sql[buff_a];
                                }
                            }
                            if(sql[buff_a]== '{')
                            {
                                justOutOfBrackets = false;
                                beginSubscriptSQL = true;
                                bracketCount++;
                                if(bracketCount > 1)
                                    searchingFirstBrackets = false;

                                if(inlineVariables)
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
                                if(bracketValue == 1)
                                {
                                    justOutOfBrackets = true;
                                }
                                if(inlineVariables)
                                    scriptCommand.push_back(' ');
                                else if(bracketValue >0)
                                    scriptCommand.push_back('}');
                                buff_a++;
                                continue;
                            }


                            if(beginSubscriptSQL && !inlineVariables)
                                scriptCommand.push_back(sql[buff_a]);


                            buff_a++;
                        }
                        a = buff_a;


                        // no recursion = manualy unroll though reset
                        if(subCommandPatterns[i].trimmed() == "ForLoop" || subCommandPatterns[i].trimmed() == "QueryForLoop" )
                        {
                            reset = true;


                            QString replValue = "_____Unset_Iter_Value_";
                            int replStart = 0;
                            int replEnd = 1;
                            int replStep = 1;
                            int fillStage = 0;
                            bool toIntStart = false;
                            QStringList iterateValues;
                            //QString valueFiller = ""; // mb implement it, for syntax like {iterval} {002} {100} and itervalue will be 3 digits, 002,003 etc.
                            int valueRuquieredSize = 0; // implemented through this. less flexability
                            bool isIteration = false;
                            for(auto s : funcVariables)
                            {
                                if(s.trimmed().size() <1)
                                    continue;
                                QString tmpvar = UnrollAllLoops(s).trimmed();
                                while(tmpvar.endsWith(QChar('\u0000')))
                                    tmpvar.resize(tmpvar.size()-1);
                                qDebug() << tmpvar << "  from  "<< s;
                                if(fillStage  == 0)
                                {
                                    fillStage++;
                                    replValue = tmpvar;
                                }else if(fillStage  == 1)
                                {
                                    fillStage++;
                                    replStart = QVariant(tmpvar).toInt(&toIntStart);
                                    valueRuquieredSize = tmpvar.size();
                                    if(!toIntStart)
                                    {
                                        iterateValues = tmpvar.split(',');
                                        if(iterateValues.size()>0)
                                            isIteration = true;
                                    }

                                }else if(fillStage  == 2)
                                {
                                    fillStage++;
                                    replEnd = QVariant(tmpvar).toInt();
                                }else if(fillStage  == 3)
                                {
                                    fillStage++;
                                    replStep = QVariant(tmpvar).toInt();
                                }
                                else
                                    break;
                            }

                            if(!isIteration)
                            {
                                qDebug() << "replStart " << replStart << "replEnd "<< replEnd << "replStep "<< replStep;
                                for(int iter = replStart; iter <= replEnd; iter += replStep)
                                {
                                    if(iter != replStart)
                                    {
                                        if(subCommandPatterns[i].trimmed() == "QueryForLoop")
                                            formatedSql+= " union all ";
                                    }
                                    QString fmiterSql = scriptCommand;
                                    QString replString = QVariant(iter).toString();
                                    while (replString.size() < valueRuquieredSize)
                                        replString = '0' + replString;

                                    fmiterSql = fmiterSql.replace(replValue, replString);
                                    qDebug() << fmiterSql;
                                    formatedSql += fmiterSql;
                                    qDebug() << formatedSql;

                                }
                            }else
                            {
                                int first = true;
                                for(auto val : iterateValues)
                                {
                                    if(!first)
                                    {
                                        if(subCommandPatterns[i].trimmed() == "QueryForLoop")
                                            formatedSql+= " union all ";
                                    }
                                    first = false;
                                    QString fmiterSql = scriptCommand;
                                    QString replString = val;

                                    fmiterSql = fmiterSql.replace(replValue, replString);
                                    formatedSql += fmiterSql;

                                }
                            }

                            formatedSql += " ";
                        }



                        if(subCommandPatterns[i].startsWith("ExcelSheetList"))
                        {
                            DatabaseConnection dc;
                            dc.data.silentExcelImport = true;
                            dc.data.ImportFromExcel(funcVariables[0],0,0,0,0,true);
                            for(auto x  : dc.data.LastExcelImportSheets)
                            {
                                formatedSql += x + ",";
                            }
                        }

                        // Async execution.
                        //// Start async - start launching each subexec on separate threads.
                        //// Wait async - wait all async executions, paste all temporary placeholders(_DBL_AsyncExecutionTempValue) with results

                        // subCommandPatterns.push_back("StartAsyncExecution");
                        // subCommandPatterns.push_back("AwaitAsyncExecution");

                        if(subCommandPatterns[i].trimmed() == "DBLPasteYearMonthOffset")
                        {
                            QString str;
                            QDateTime::currentDateTime().date().month();
                            int monthnum = QDateTime::currentDateTime().date().daysInMonth();
                            if(funcVariables.size() > 0)
                            {
                                int offset = 0;
                                bool ok = false;
                                offset = QVariant(funcVariables[0]).toInt(&ok);
                                if(ok)
                                    monthnum = QDateTime::currentDateTime().date().addMonths(offset).year();
                            }
                            str = QVariant(monthnum).toString();

                            formatedSql += str;
                        }

                        if(subCommandPatterns[i].trimmed() == "DBLPasteYearDayOffset")
                        {
                            QString str;
                            QDateTime::currentDateTime().date().month();
                            int monthnum = QDateTime::currentDateTime().date().daysInMonth();
                            if(funcVariables.size() > 0)
                            {
                                int offset = 0;
                                bool ok = false;
                                offset = QVariant(funcVariables[0]).toInt(&ok);
                                if(ok)
                                    monthnum = QDateTime::currentDateTime().date().addDays(offset).year();
                            }
                            str = QVariant(monthnum).toString();

                            formatedSql += str;
                        }


                        if(subCommandPatterns[i].trimmed() == "DBLPasteDaysInMonth")
                        {
                            QString str;
                            QDateTime::currentDateTime().date().month();
                            int monthnum = QDateTime::currentDateTime().date().daysInMonth();
                            if(funcVariables.size() > 0)
                            {
                                int offset = 0;
                                bool ok = false;
                                offset = QVariant(funcVariables[0]).toInt(&ok);
                                if(ok)
                                    monthnum = QDateTime::currentDateTime().date().addMonths(offset).daysInMonth();
                            }
                            str = QVariant(monthnum).toString();

                            formatedSql += str;
                        }
                        if(subCommandPatterns[i].trimmed() == "DBLPasteDaysInMonthDayOffset")
                        {
                            QString str;
                            QDateTime::currentDateTime().date().month();
                            int monthnum = QDateTime::currentDateTime().date().daysInMonth();
                            if(funcVariables.size() > 0)
                            {
                                int offset = 0;
                                bool ok = false;
                                offset = QVariant(funcVariables[0]).toInt(&ok);
                                if(ok)
                                    monthnum = QDateTime::currentDateTime().date().addDays(offset).daysInMonth();
                            }
                            str = QVariant(monthnum).toString();

                            formatedSql += str;
                        }

                        if(subCommandPatterns[i].trimmed() == "DBLPasteDaysInMonthByMonth")
                        {
                            QString str;
                            QDate dt = QDate::currentDate();
                            dt.setDate(QDateTime::currentDateTime().date().year(),1,1);
                            int monthnum = dt.daysInMonth();
                            if(funcVariables.size() > 0)
                            {
                                int offset = 0;
                                bool ok = false;
                                offset = QVariant(funcVariables[0]).toInt(&ok);
                                if(ok)
                                {
                                    dt.setDate(QDate::currentDate().year(),offset,1);
                                    monthnum = dt.daysInMonth();
                                }


                                if(funcVariables.size() > 1)
                                {
                                    int offset2 = QVariant(funcVariables[1]).toInt(&ok);
                                    if(ok)
                                    {
                                        dt.setDate(offset2,offset,1);
                                        monthnum = dt.daysInMonth();
                                    }
                                }
                            }
                            str = QVariant(monthnum).toString();

                            formatedSql += str;
                        }


                        if(subCommandPatterns[i].trimmed() == "DBLPasteDayOfMonth")
                        {
                            QString str;
                            QDateTime::currentDateTime().date().month();
                            int monthnum = QDateTime::currentDateTime().date().day();
                            if(funcVariables.size() > 0)
                            {
                                int offset = 0;
                                bool ok = false;
                                offset = QVariant(funcVariables[0]).toInt(&ok);
                                if(ok)
                                    monthnum = QDateTime::currentDateTime().date().addDays(offset).day();
                            }
                            str = QVariant(monthnum).toString();

                            formatedSql += str;
                        }


                        if(subCommandPatterns[i].trimmed() == "DBLPasteMonthDayOffset")
                        {
                            QString str;
                            QDateTime::currentDateTime().date().month();
                            int monthnum = QDateTime::currentDateTime().date().month();
                            if(funcVariables.size() > 0)
                            {
                                int offset = 0;
                                bool ok = false;
                                offset = QVariant(funcVariables[0]).toInt(&ok);
                                if(ok)
                                    monthnum = QDateTime::currentDateTime().date().addDays(offset).month();
                            }
                            str = QVariant(monthnum).toString();
                            if(str.size() == 1)
                                str = "0" + str;

                            formatedSql += str;
                        }

                        if(subCommandPatterns[i].trimmed() == "DBLPasteNumMonthDayOffset")
                        {
                            QString str;
                            QDateTime::currentDateTime().date().month();
                            int monthnum = QDateTime::currentDateTime().date().month();
                            if(funcVariables.size() > 0)
                            {
                                int offset = 0;
                                bool ok = false;
                                offset = QVariant(funcVariables[0]).toInt(&ok);
                                if(ok)
                                    monthnum = QDateTime::currentDateTime().date().addDays(offset).month();
                            }
                            str = QVariant(monthnum).toString();

                            formatedSql += str;
                        }
                        if(subCommandPatterns[i].trimmed() == "DBLPasteMonth")
                        {
                            QString str;
                            QDateTime::currentDateTime().date().month();
                            int monthnum = QDateTime::currentDateTime().date().month();
                            if(funcVariables.size() > 0)
                            {
                                int offset = 0;
                                bool ok = false;
                                offset = QVariant(funcVariables[0]).toInt(&ok);
                                if(ok)
                                    monthnum = QDateTime::currentDateTime().date().addMonths(offset).month();
                            }
                            str = QVariant(monthnum).toString();
                            if(str.size() == 1)
                                str = "0" + str;

                            formatedSql += str;
                        }

                        if(subCommandPatterns[i].trimmed() == "DBLPasteNumMonth")
                        {
                            QString str;
                            QDateTime::currentDateTime().date().month();
                            int monthnum = QDateTime::currentDateTime().date().month();
                            if(funcVariables.size() > 0)
                            {
                                int offset = 0;
                                bool ok = false;
                                offset = QVariant(funcVariables[0]).toInt(&ok);
                                if(ok)
                                    monthnum = QDateTime::currentDateTime().date().addMonths(offset).month();
                            }
                            str = QVariant(monthnum).toString();

                            formatedSql += str;
                        }
                        qDebug() << "keywordbuff "<<keywordbuff  ;
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
        sql = formatedSql;
    }
    sql = formatedSql.trimmed();
    return sql;
}

bool DatabaseConnection::execSql(QString sql)
{
    stopNow = false;// to be shure we wont reject query right after exec
    lastLaunchIsError = false;
    dataDownloading = false;
    lastErrorPos = -1;
    savefilecount = 0;
    queryExecutionState = 0;
    tableDataMutex.lock();
    tableDataMutex.unlock();
    executionTime = QDateTime::currentSecsSinceEpoch();
    executionStart = QDateTime::currentDateTime();

    data.exported = false;
    if(!db.isOpen() && !customOracle && (!(customSQlite)) && !(customPSQL))
    {
        bool ok = db.open();
        if(ok)
        {
            if(!nodebug) qDebug() << "db opened";
        }
        else
        {
            if(!nodebug) qDebug() << "nope: "<< Last_ConnectError;

            executionTime = executionEnd.toSecsSinceEpoch() - executionStart.toSecsSinceEpoch();
            executionEnd = QDateTime::currentDateTime();
            if(!nodebug) qDebug() << "Execd";
            executing = false;
            queryExecutionState = 4;

            emit execedSql();
            return true;
        }
    }


    if(sql.size() <=0)
        sql = sqlCode;
    QString str = sql;
    this->data.allSqlCode = str;




    if(!rawquery)
    {
        sql = UnrollAllLoops(sql);
        // Prepeare sql to be run, detect any special keywords
        QString formatedSql;
        QString keywordbuff = "";
        std::vector<int> localKeywordsMatch;
        localKeywordsMatch.resize(subCommandPatterns.size());

        // iterate through keywords, detect similarity
        if(!nodebug) qDebug()<<"Entered sql subcomand processing ";
        bool reset = true;
        int loopcount = -1;

        bool asyncExecution_recordExecs = false;
        bool asyncExecution_pasteExecsData = false;
        QList<QStringList> asyncExecution_Formats;
        QList<int> asyncExecution_commandIds;
        QList<QThread*> asyncExecution_threads;
        asyncExecution_databaseConnections.clear();


        // loop while keywords can be possible in code (due to nested loops and other syntax)
        while (reset)
        {
            formatedSql = "";
            keywordbuff = "";
            localKeywordsMatch.clear();
            localKeywordsMatch.resize(subCommandPatterns.size());
            loopcount++;
            reset = false;
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


                            bool good = false;
                            for(int it = a +1; it < sql.size();it++)
                            {
                                if(sql[it] == '{')
                                {
                                    good = true;
                                    break;
                                }
                                else if(sql[it] == ' ' || sql[it] == "   ")
                                    continue;
                                else break;
                            }
                            if(!good)
                            {
                                localKeywordsMatch[i] = 0;
                                continue; // break out of commands loop, to go to next char
                            }


                            keywordbuff += sql[a];
                            while(localKeywordsMatch[i] < keywordbuff.size())
                            {
                                formatedSql+= keywordbuff[0];
                                keywordbuff.remove(0,1);
                            }

                            if(!nodebug) qDebug()<<"Detected keyword \"" << subCommandPatterns[i] <<"\"";
                            localKeywordsMatch[i] = 0;
                            scriptCommand = subCommandPatterns[i];
                            int buff_a = a;
                            bool beginSubscriptSQL = false;
                            int bracketValue = 0;
                            int bracketCount = 0;
                            bool inlineVariables = true;
                            bool justOutOfBrackets = false;
                            bool searchingFirstBrackets = false;
                            QStringList funcVariables;
                            QString lastVar = "";
                            scriptCommand = "";
                            while(buff_a < sql.size())
                            {
                                if(bracketValue <=0 && beginSubscriptSQL)
                                {
                                    beginSubscriptSQL = false;
                                    a = buff_a;
                                    break;
                                }
                                if(justOutOfBrackets  && sql[buff_a] != '{'&& sql[buff_a] != '}'&& sql[buff_a] != ' '&& sql[buff_a] != '\t'&& sql[buff_a] != '\n'&& sql[buff_a] != '\t'&& sql[buff_a] != '\r')
                                    inlineVariables = false;
                                if(inlineVariables)
                                {

                                    if(sql[buff_a] == '}' && bracketValue < 3)
                                    {
                                        lastVar = lastVar.trimmed();
                                        funcVariables  << lastVar;
                                        lastVar = "";
                                    }
                                    else if(bracketValue > 1 && sql[buff_a] != '\t'&& sql[buff_a] != '\t'&& sql[buff_a] != '\r')
                                    {
                                        lastVar += sql[buff_a];
                                    }
                                }
                                if(sql[buff_a]== '{')
                                {
                                    justOutOfBrackets = false;
                                    beginSubscriptSQL = true;
                                    bracketCount++;
                                    if(bracketCount > 1)
                                        searchingFirstBrackets = false;

                                    if(inlineVariables)
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
                                    if(bracketValue == 1)
                                    {
                                        justOutOfBrackets = true;
                                    }
                                    if(inlineVariables)
                                        scriptCommand.push_back(' ');
                                    else if(bracketValue >0)
                                        scriptCommand.push_back('}');
                                    buff_a++;
                                    continue;
                                }


                                if(beginSubscriptSQL && !inlineVariables)
                                    scriptCommand.push_back(sql[buff_a]);


                                buff_a++;
                            }
                            a = buff_a;

                            if(subCommandPatterns[i].startsWith("_DBL_AsyncExecutionTempValue"))
                            {
                                funcVariables = asyncExecution_Formats[0];
                                asyncExecution_Formats.pop_front();

                                i = asyncExecution_commandIds[0];
                                asyncExecution_commandIds.pop_front();
                                qDebug() << "Async function paste of: " << subCommandPatterns[i] << " vars:" << funcVariables;
                                asyncExecution_pasteExecsData = true;
                            }
                            else
                                asyncExecution_pasteExecsData = false;


                            if(!nodebug) qDebug()<< "subexec variables " << funcVariables;
                            // recursion does its thing, recursive subexec's are possible and infinite^tm
                            if(subCommandPatterns[i].startsWith("Subexec") ||subCommandPatterns[i].startsWith("SilentSubexec") || subCommandPatterns[i].startsWith("ExcelTo")|| subCommandPatterns[i].startsWith("ExcelSheetList")|| subCommandPatterns[i].startsWith("CSVTo"))
                            {// its a subexec, crop part from next { to }, exec it in subscriptConnection
                                if(!nodebug) qDebug()<<"Its SubExec or ExcelTo or CSVTo";

                                // "_DBL_AsyncExecutionTempValue";
                                // asyncExecution_Formats;
                                // asyncExecution_commandIds;
                                // asyncExecution_threads;
                                // asyncExecution_databaseConnections;

                                if(!nodebug) qDebug()<< "";
                                if(!nodebug) qDebug()<< "Subscript command is:";
                                if(!nodebug) qDebug()<< scriptCommand;
                                if(!nodebug) qDebug()<< "";
                                QString subscriptDriver = "NoDriver";
                                QString subscriptDBname = "NoDatabase";
                                QString saveName = "unset_tmp_savename";
                                QString WorksheetName = "Sheet1";
                                char csvDelimeter = ';';
                                int saveStart_X = 0;
                                int saveStart_Y = 0;
                                int saveEnd_X = 0;
                                int saveEnd_Y = 0;

                                int varcount = 0;
                                for(auto s : funcVariables)
                                {
                                    if(s.trimmed().size() <1)
                                        continue;
                                    QString tmpvar = UnrollAllLoops(s).trimmed();
                                    while(tmpvar.endsWith(QChar('\u0000')))
                                        tmpvar.resize(tmpvar.size()-1);

                                    if(varcount == 0)
                                    {
                                        varcount++;
                                        subscriptDriver = tmpvar;
                                    }else if(varcount == 1)
                                    {
                                        varcount++;
                                        subscriptDBname = tmpvar;
                                    }
                                    else if(varcount == 2)
                                    {
                                        varcount++;
                                        saveName = tmpvar;
                                    }
                                    else if(varcount == 3)
                                    {
                                        varcount++;
                                        saveStart_X = QVariant(tmpvar).toInt();
                                        csvDelimeter = tmpvar[0].unicode();
                                        WorksheetName=tmpvar;
                                        if(subCommandPatterns[i].endsWith("ExcelWorksheet"))
                                        {
                                            WorksheetName=tmpvar;
                                        }
                                    }
                                    else if(varcount == 4)
                                    {
                                        varcount++;
                                        saveEnd_X = QVariant(tmpvar).toInt();
                                        if(subCommandPatterns[i].endsWith("ExcelWorksheet"))
                                        {
                                            saveStart_X = QVariant(tmpvar).toInt();
                                        }
                                    }
                                    else if(varcount == 5)
                                    {
                                        varcount++;
                                        saveStart_Y = QVariant(tmpvar).toInt();
                                        if(subCommandPatterns[i].endsWith("ExcelWorksheet"))
                                        {
                                            saveEnd_X = QVariant(tmpvar).toInt();
                                        }
                                    }
                                    else if(varcount == 6)
                                    {
                                        varcount++;
                                        saveEnd_Y = QVariant(tmpvar).toInt();
                                        if(subCommandPatterns[i].endsWith("ExcelWorksheet"))
                                        {
                                            saveStart_Y = QVariant(tmpvar).toInt();
                                        }
                                    }
                                    else if(varcount == 7)
                                    {
                                        if(!subCommandPatterns[i].endsWith("ExcelWorksheet"))
                                        {
                                            WorksheetName=tmpvar;
                                        }
                                        else
                                        {
                                            saveEnd_Y = QVariant(tmpvar).toInt();
                                        }

                                    }
                                    else
                                        break;
                                }

                                if(!nodebug) qDebug()<< "subscriptDriver: " << subscriptDriver;
                                if(!nodebug) qDebug()<< "subscriptDBname: " << subscriptDBname;

                                if(asyncExecution_pasteExecsData)
                                {
                                    subscriptConnesction = asyncExecution_databaseConnections[0];
                                    asyncExecution_databaseConnections.pop_front();
                                }
                                else
                                    subscriptConnesction = new DatabaseConnection();

                                if(!nodebug) qDebug()<< "created DatabaseConnection";


                                // subCommandPatterns.push_back("ExcelToMagic");
                                // subCommandPatterns.push_back("ExcelToArray");
                                // if its subexec, subexec, else load from excel file
                                if(!asyncExecution_pasteExecsData)
                                    if(subCommandPatterns[i].startsWith("Subexec") ||subCommandPatterns[i].startsWith("SilentSubexec"))
                                    {
                                        QString username = userDS.GetObject(subscriptDBname)["name"];
                                        QString password = userDS.GetObject(subscriptDBname)["password"];
                                        if(!nodebug) qDebug()<< subscriptDriver << subscriptDBname << username.trimmed() << password.trimmed();
                                        subscriptConnesction->connectionName = connectionName + QVariant(a).toString();
                                        if(!nodebug) qDebug()<< "creating actual Connection";
                                        subscriptConnesction->Create(subscriptDriver.trimmed(),subscriptDBname.trimmed(),username.trimmed(),password.trimmed());
                                        if(!nodebug) qDebug()<< "created";
                                        while(scriptCommand.endsWith(' ') ||scriptCommand.endsWith('\t') ||scriptCommand.endsWith('\n') || scriptCommand.endsWith('\r'))
                                            scriptCommand.resize(scriptCommand.size()-1);

                                        if(asyncExecution_recordExecs)
                                        {

                                            asyncExecution_Formats.push_back(funcVariables);
                                            asyncExecution_databaseConnections.push_back(subscriptConnesction);
                                            subscriptConnesction->nodebug = true;
                                            subscriptConnesction->disableSaveToUserDS = true;
                                            asyncExecution_commandIds.push_back(i);
                                            asyncExecution_threads.push_back(QThread::create(_dc_AsyncFunc,subscriptConnesction,scriptCommand.trimmed()));
                                            asyncExecution_threads.back()->start();
                                            keywordbuff = "";
                                            formatedSql+= "_DBL_AsyncExecutionTempValue {{}}";//{-- {} {} \n}
                                            reset = true;
                                            break;

                                        }
                                        else if (!reset)
                                        {
                                            if(!nodebug) qDebug()<< "executing sql";
                                            subscriptConnesction->execSql(scriptCommand.trimmed());
                                            subscriptConnesction->data.sqlCode = scriptCommand.trimmed();
                                            if(!nodebug) qDebug()<< "sql execd";
                                        }
                                        else
                                        {
                                            formatedSql+= keywordbuff;
                                            formatedSql+= "{";

                                            for(auto s : funcVariables)
                                                formatedSql+= "{" + s + "}";
                                            formatedSql+= "\n";

                                            formatedSql+= scriptCommand.trimmed();

                                            formatedSql+= "}";
                                            keywordbuff = "";
                                            break;
                                        }
                                    }
                                subscriptConnesction->data.allSqlCode = this->data.allSqlCode;


                                QString hours =  QVariant((subscriptConnesction->executionTime / 3600)).toString();
                                QString minuts = QVariant(subscriptConnesction->executionTime % 3600 / 60).toString();
                                QString secs = QVariant(subscriptConnesction->executionTime % 60).toString();
                                QString msecs = QVariant((QDateTime::currentMSecsSinceEpoch() - subscriptConnesction->executionStart.toMSecsSinceEpoch())%1000).toString();

                                QString execTimeStr =  "";
                                while(hours.size() <2)
                                    hours = QString("0") + hours;
                                while(minuts.size() <2)
                                    minuts = QString("0") + minuts;
                                while(secs.size() <2)
                                    secs = QString("0") + secs;
                                while(msecs.size() <3)
                                    msecs = QString("0") + msecs;
                                execTimeStr += hours;
                                execTimeStr += ":";
                                execTimeStr += minuts;
                                execTimeStr += ":";
                                execTimeStr += secs;
                                execTimeStr += ".";
                                execTimeStr += msecs;




                                int neededMagicColumn = -1;

                                int TableCutoffStart = -1;
                                int TableCutoffEnd = -1;

                                if(subCommandPatterns[i].startsWith("ExcelTo"))
                                {
                                    neededMagicColumn=0;// autoselect column 1, if no column selected
                                    // cut autopaste part of path

                                    if(saveName != "unset_tmp_savename" && saveStart_X!=0)
                                    {
                                        bool ok = false;
                                        TableCutoffStart = QVariant(saveName).toInt(&ok);

                                        if(!ok || TableCutoffStart <0)
                                        {
                                            TableCutoffStart =-1;
                                            TableCutoffEnd =-1;
                                        }
                                        else
                                        {
                                            // all good
                                            TableCutoffEnd = TableCutoffStart + saveStart_X;
                                        }
                                    }
                                    if(subscriptDriver.startsWith("file:///"))
                                    {
                                        for(int i=0;i < subscriptDriver.size();i++)
                                        {
                                            if(i+7 < subscriptDriver.size())
                                                subscriptDriver[i]=subscriptDriver[i+8];
                                        }
                                        subscriptDriver.resize(subscriptDriver.size()-8);
                                        if(!nodebug) qDebug() <<"subscriptDriver" <<subscriptDriver;
                                    }
                                    subscriptConnesction->data.silentExcelImport = true;
                                    subscriptConnesction->data.ImportFromExcel(subscriptDriver,0,0,0,0,true,funcVariables[2].trimmed());
                                    qDebug() << "WorksheetName: "<< funcVariables[2].trimmed();
                                    qDebug() << "subscriptDriver: "<< subscriptDriver;
                                    qDebug() << "subscriptDBname: "<< subscriptDBname;
                                    if(subscriptDBname != "NoDatabase")
                                    {// leave only one column
                                        for(int it=0;it<subscriptConnesction->data.headers.size();it++)
                                        {
                                            if(subscriptConnesction->data.headers[it] == subscriptDBname)
                                            {
                                                neededMagicColumn = it;
                                                break;
                                            }
                                        }
                                    }
                                }
                                if(subCommandPatterns[i].startsWith("CSVTo"))
                                {
                                    neededMagicColumn=0;// autoselect column 1, if no column selected
                                    // cut autopaste part of path

                                    if(saveName != "unset_tmp_savename" && saveStart_X!=0)
                                    {
                                        bool ok = false;
                                        TableCutoffStart = QVariant(saveName).toInt(&ok);

                                        if(!ok || TableCutoffStart <0)
                                        {
                                            TableCutoffStart =-1;
                                            TableCutoffEnd =-1;
                                        }
                                        else
                                        {
                                            // all good
                                            TableCutoffEnd = TableCutoffStart + saveStart_X;
                                        }
                                    }
                                    if(subscriptDriver.startsWith("file:///"))
                                    {
                                        for(int i=0;i < subscriptDriver.size();i++)
                                        {
                                            if(i+7 < subscriptDriver.size())
                                                subscriptDriver[i]=subscriptDriver[i+8];
                                        }
                                        subscriptDriver.resize(subscriptDriver.size()-8);
                                        if(!nodebug) qDebug() <<"subscriptDriver" <<subscriptDriver;
                                    }

                                    subscriptConnesction->data.ImportFromCSV(subscriptDriver,';',true);
                                    if(subscriptDBname != "NoDatabase")
                                    {// leave only one column
                                        for(int it=0;it<subscriptConnesction->data.headers.size();it++)
                                        {
                                            if(subscriptConnesction->data.headers[it] == subscriptDBname)
                                            {
                                                neededMagicColumn = it;
                                                break;
                                            }
                                        }
                                    }
                                }




                                // silent exporting
                                if(subCommandPatterns[i] == "SilentSubexecToSqliteTable" || subCommandPatterns[i] == "SilentSubexecToLocalDBTable")
                                {
                                    savefilecount++;
                                    if(!nodebug) qDebug() << "silent exporting to SQLite " << saveName;

                                    if(subscriptConnesction->data.headers.size() > 0 && !subscriptConnesction->data.headers[0].startsWith("Error"))
                                        if(subscriptConnesction != nullptr && subscriptConnesction->data.tbldata.size() > 0)
                                            subscriptConnesction->data.ExportToSQLiteTable(saveName);
                                }
                                else if(subCommandPatterns[i] == "SilentSubexecToExcelTable")
                                {
                                    savefilecount++;
                                    if(!nodebug) qDebug() << "silent exporting to Excel " << saveName;

                                    if(subscriptConnesction->data.headers.size() > 0 && !subscriptConnesction->data.headers[0].startsWith("Error"))
                                        if(subscriptConnesction != nullptr && subscriptConnesction->data.tbldata.size() > 0)
                                            subscriptConnesction->data.ExportToExcel(QString(documentsDir + "/" + "excel/") + QString(saveName) + QString(".xlsx"),saveStart_X,saveEnd_X,saveStart_Y,saveEnd_Y,true);
                                }
                                else if(subCommandPatterns[i] == "SilentSubexecToExcelWorksheet")
                                {
                                    savefilecount++;
                                    if(!nodebug) qDebug() << "silent exporting to Excel Worksheet" << saveName;

                                    if(subscriptConnesction->data.headers.size() > 0 && !subscriptConnesction->data.headers[0].startsWith("Error"))
                                        if(subscriptConnesction != nullptr && subscriptConnesction->data.tbldata.size() > 0)
                                            subscriptConnesction->data.ExportToExcel(QString(documentsDir + "/" + "excel/") + QString(saveName) + QString(".xlsx"),saveStart_X,saveEnd_X,saveStart_Y,saveEnd_Y,true,WorksheetName);
                                }
                                else if(subCommandPatterns[i] == "SilentSubexecAppendExcelWorksheet")
                                {
                                    savefilecount++;
                                    if(!nodebug) qDebug() << "silent exporting to Excel Worksheet" << saveName;

                                    if(subscriptConnesction->data.headers.size() > 0 && !subscriptConnesction->data.headers[0].startsWith("Error"))
                                        if(subscriptConnesction != nullptr && subscriptConnesction->data.tbldata.size() > 0)
                                            subscriptConnesction->data.ExportToExcel(QString(documentsDir + "/" + "excel/") + QString(saveName) + QString(".xlsx"),saveStart_X,saveEnd_X,saveStart_Y,saveEnd_Y,true,WorksheetName,true);
                                }

                                else if(subCommandPatterns[i] == "SilentSubexecToCSV")
                                {
                                    savefilecount++;
                                    if(!nodebug) qDebug() << "silent exporting to CSV " << saveName;
                                    if(subscriptConnesction->data.headers.size() > 0 && !subscriptConnesction->data.headers[0].startsWith("Error"))
                                        if(subscriptConnesction != nullptr && subscriptConnesction->data.tbldata.size() > 0)
                                            subscriptConnesction->data.ExportToCSV(QString(documentsDir + "/" +"CSV/") + QString(saveName) + QString(".csv"),csvDelimeter,true);
                                }
                                else if(subCommandPatterns[i] == "SilentSubexecAppendCSV")
                                {
                                    savefilecount++;
                                    if(!nodebug) qDebug() << "silent exporting(Append) to CSV " << saveName;
                                    if(subscriptConnesction->data.headers.size() > 0 && !subscriptConnesction->data.headers[0].startsWith("Error"))
                                        if(subscriptConnesction != nullptr && subscriptConnesction->data.tbldata.size() > 0)
                                            subscriptConnesction->data.AppendToCSV(QString(documentsDir + "/" +"CSV/") + QString(saveName) + QString(".csv"),csvDelimeter);
                                }

                                // non silent exporting
                                else if(subCommandPatterns[i] == "SubexecToSqliteTable" || subCommandPatterns[i] == "SubexecToLocalDBTable")
                                {
                                    savefilecount++;
                                    if(!nodebug) qDebug() << "exporting to SQLite " << saveName;
                                    if(subscriptConnesction != nullptr && subscriptConnesction->data.tbldata.size() > 0)
                                    {
                                        QString saveErrorStr = "";
                                        if(subscriptConnesction->data.headers.size() > 0 && subscriptConnesction->data.headers[0] != "Error")
                                            if(!subscriptConnesction->data.ExportToSQLiteTable(saveName))
                                                saveErrorStr = "Failed to save to SQLite table, check columnName repetitions/spaces, special symbols";
                                        if(subscriptConnesction->data.headers.size() > 0 && subscriptConnesction->data.headers[0] == "Error")
                                            formatedSql += " 'Error' as \"Status\", ";
                                        else if (saveErrorStr.size() > 0)
                                        {
                                            formatedSql += " '";
                                            formatedSql += saveErrorStr;
                                            formatedSql += "' ";
                                            formatedSql += "as \"Status\", ";
                                        }
                                        else
                                            formatedSql += " 'Success' as \"Status\", ";

                                        if(subscriptConnesction->data.headers.size() > 0 && subscriptConnesction->data.headers[0] == "Error")
                                        {
                                            formatedSql += "'";
                                            if(subscriptConnesction->data.tbldata.size() > 0 && subscriptConnesction->data.tbldata[0].size()>0)
                                                formatedSql += subscriptConnesction->data.tbldata[0][0];
                                            formatedSql += "'";
                                            formatedSql += " as \"Error message\", ";
                                        }
                                        else
                                        {
                                            formatedSql += "'";
                                            formatedSql += " ";
                                            formatedSql += "'";
                                            formatedSql += " as \"Error message\", ";
                                        }
                                        formatedSql += "'";
                                        formatedSql += QVariant(subscriptConnesction->data.tbldata.size()).toString();
                                        formatedSql += "'";
                                        formatedSql += " as \"Column count\", ";
                                        formatedSql += "'";
                                        if(subscriptConnesction->data.tbldata.size() > 0)
                                            formatedSql += QVariant(subscriptConnesction->data.tbldata[0].size()).toString();
                                        else
                                            formatedSql += " 0 ";
                                        formatedSql += "'";
                                        formatedSql += " as \"Row count\" ";


                                        formatedSql += ", '";
                                        formatedSql += execTimeStr;
                                        formatedSql += "' ";
                                        formatedSql += "as \"Execution time\"";
                                    }
                                }
                                else if(subCommandPatterns[i] == "SubexecToExcelTable")
                                {
                                    savefilecount++;
                                    if(!nodebug) qDebug() << "exporting to Excel " << saveName;
                                    QString saveErrorStr = "";
                                    if(subscriptConnesction->data.headers.size() > 0 && !subscriptConnesction->data.headers[0].startsWith("Error"))
                                        if(subscriptConnesction != nullptr && subscriptConnesction->data.tbldata.size() > 0 && subscriptConnesction->data.tbldata[0].size() > 0)
                                            if(!subscriptConnesction->data.ExportToExcel(QString(documentsDir + "/" +"excel/") + QString(saveName) + QString(".xlsx"),saveStart_X,saveEnd_X,saveStart_Y,saveEnd_Y,true))
                                                saveErrorStr = "Failed to save to excel, probably file is opened";



                                    if (saveErrorStr.size() > 0)
                                    {
                                        formatedSql += " '";
                                        formatedSql += saveErrorStr;
                                        formatedSql += "' ";
                                        formatedSql += "as \"Status\", ";
                                    }
                                    else
                                        formatedSql += " 'Success' as \"Status\", ";

                                    if(subscriptConnesction->data.headers.size() > 0 && !subscriptConnesction->data.headers[0].startsWith("Error"))
                                    {
                                        formatedSql += "'";
                                        if(subscriptConnesction->data.tbldata.size() > 0 && subscriptConnesction->data.tbldata[0].size()>0)
                                            formatedSql += subscriptConnesction->data.tbldata[0][0];
                                        formatedSql += "'";
                                        formatedSql += " as \"Error message\", ";
                                    }
                                    else
                                    {
                                        formatedSql += "'";
                                        formatedSql += " ";
                                        formatedSql += "'";
                                        formatedSql += " as \"Error message\", ";
                                    }
                                    formatedSql += "'";
                                    formatedSql += QVariant(subscriptConnesction->data.tbldata.size()).toString();
                                    formatedSql += "'";
                                    formatedSql += " as \"Column count\", ";
                                    formatedSql += "'";
                                    if(subscriptConnesction->data.tbldata.size() > 0)
                                        formatedSql += QVariant(subscriptConnesction->data.tbldata[0].size()).toString();
                                    else
                                        formatedSql += " 0 ";
                                    formatedSql += "'";
                                    formatedSql += " as \"Row count\" ";


                                    formatedSql += ", '";
                                    formatedSql += execTimeStr;
                                    formatedSql += "' ";
                                    formatedSql += "as \"Execution time\"";
                                }
                                else if(subCommandPatterns[i] == "SubexecToExcelWorksheet")
                                {
                                    savefilecount++;
                                    if(!nodebug) qDebug() << "exporting to Excel " << saveName;
                                    QString saveErrorStr = "";
                                    if(subscriptConnesction->data.headers.size() > 0 && !subscriptConnesction->data.headers[0].startsWith("Error"))
                                        if(subscriptConnesction != nullptr && subscriptConnesction->data.tbldata.size() > 0 && subscriptConnesction->data.tbldata[0].size() > 0)
                                            if(!subscriptConnesction->data.ExportToExcel(QString(documentsDir + "/" +"excel/") + QString(saveName) + QString(".xlsx"),saveStart_X,saveEnd_X,saveStart_Y,saveEnd_Y,true,WorksheetName))
                                                saveErrorStr = "Failed to save to excel, probably file is opened";

                                    if (saveErrorStr.size() > 0)
                                    {
                                        formatedSql += " '";
                                        formatedSql += saveErrorStr;
                                        formatedSql += "' ";
                                        formatedSql += "as \"Status\", ";
                                    }
                                    else
                                        formatedSql += " 'Success' as \"Status\", ";

                                    if(subscriptConnesction->data.headers.size() > 0 && !subscriptConnesction->data.headers[0].startsWith("Error"))
                                    {
                                        formatedSql += "'";
                                        if(subscriptConnesction->data.tbldata.size() > 0 && subscriptConnesction->data.tbldata[0].size()>0)
                                            formatedSql += subscriptConnesction->data.tbldata[0][0];
                                        formatedSql += "'";
                                        formatedSql += " as \"Error message\", ";
                                    }
                                    else
                                    {
                                        formatedSql += "'";
                                        formatedSql += " ";
                                        formatedSql += "'";
                                        formatedSql += " as \"Error message\", ";
                                    }
                                    formatedSql += "'";
                                    formatedSql += QVariant(subscriptConnesction->data.tbldata.size()).toString();
                                    formatedSql += "'";
                                    formatedSql += " as \"Column count\", ";
                                    formatedSql += "'";
                                    if(subscriptConnesction->data.tbldata.size() > 0)
                                        formatedSql += QVariant(subscriptConnesction->data.tbldata[0].size()).toString();
                                    else
                                        formatedSql += " 0 ";
                                    formatedSql += "'";
                                    formatedSql += " as \"Row count\" ";


                                    formatedSql += ", '";
                                    formatedSql += execTimeStr;
                                    formatedSql += "' ";
                                    formatedSql += "as \"Execution time\"";
                                }
                                else if(subCommandPatterns[i] == "SubexecAppendExcelWorksheet")
                                {
                                    savefilecount++;
                                    if(!nodebug) qDebug() << "exporting to Excel " << saveName;
                                    QString saveErrorStr = "";
                                    if(subscriptConnesction->data.headers.size() > 0 && !subscriptConnesction->data.headers[0].startsWith("Error"))
                                        if(subscriptConnesction != nullptr && subscriptConnesction->data.tbldata.size() > 0 && subscriptConnesction->data.tbldata[0].size() > 0)
                                            if(!subscriptConnesction->data.ExportToExcel(QString(documentsDir + "/" +"excel/") + QString(saveName) + QString(".xlsx"),saveStart_X,saveEnd_X,saveStart_Y,saveEnd_Y,true,WorksheetName,true))
                                                saveErrorStr = "Failed to save to excel, probably file is opened";

                                    if (saveErrorStr.size() > 0)
                                    {
                                        formatedSql += " '";
                                        formatedSql += saveErrorStr;
                                        formatedSql += "' ";
                                        formatedSql += "as \"Status\", ";
                                    }
                                    else
                                        formatedSql += " 'Success' as \"Status\", ";

                                    if(subscriptConnesction->data.headers.size() > 0 && !subscriptConnesction->data.headers[0].startsWith("Error"))
                                    {
                                        formatedSql += "'";
                                        if(subscriptConnesction->data.tbldata.size() > 0 && subscriptConnesction->data.tbldata[0].size()>0)
                                            formatedSql += subscriptConnesction->data.tbldata[0][0];
                                        formatedSql += "'";
                                        formatedSql += " as \"Error message\", ";
                                    }
                                    else
                                    {
                                        formatedSql += "'";
                                        formatedSql += " ";
                                        formatedSql += "'";
                                        formatedSql += " as \"Error message\", ";
                                    }
                                    formatedSql += "'";
                                    formatedSql += QVariant(subscriptConnesction->data.tbldata.size()).toString();
                                    formatedSql += "'";
                                    formatedSql += " as \"Column count\", ";
                                    formatedSql += "'";
                                    if(subscriptConnesction->data.tbldata.size() > 0)
                                        formatedSql += QVariant(subscriptConnesction->data.tbldata[0].size()).toString();
                                    else
                                        formatedSql += " 0 ";
                                    formatedSql += "'";
                                    formatedSql += " as \"Row count\" ";


                                    formatedSql += ", '";
                                    formatedSql += execTimeStr;
                                    formatedSql += "' ";
                                    formatedSql += "as \"Execution time\"";
                                }
                                else if(subCommandPatterns[i] == "SubexecToCSV")
                                {
                                    savefilecount++;
                                    if(!nodebug) qDebug() << "exporting to CSV " << saveName;
                                    if(subscriptConnesction->data.headers.size() > 0 && !subscriptConnesction->data.headers[0].startsWith("Error"))
                                        if(subscriptConnesction != nullptr && subscriptConnesction->data.tbldata.size() > 0)
                                            subscriptConnesction->data.ExportToCSV(QString(documentsDir + "/" +"CSV/") + QString(saveName) + QString(".csv"),csvDelimeter,true);
                                    if(subscriptConnesction->data.headers.size() > 0 && subscriptConnesction->data.headers[0] == "Error")
                                        formatedSql += " 'Error' as \"Status\", ";
                                    else
                                        formatedSql += " 'Success' as \"Status\", ";

                                    if(subscriptConnesction->data.headers.size() > 0 && subscriptConnesction->data.headers[0] == "Error")
                                    {
                                        formatedSql += "'";
                                        formatedSql += subscriptConnesction->data.tbldata[0][0];
                                        formatedSql += "'";
                                        formatedSql += " as \"Error message\", ";
                                    }
                                    else
                                    {
                                        formatedSql += "'";
                                        formatedSql += " ";
                                        formatedSql += "'";
                                        formatedSql += " as \"Error message\", ";
                                    }
                                    formatedSql += "'";
                                    formatedSql += QVariant(subscriptConnesction->data.tbldata.size()).toString();
                                    formatedSql += "'";
                                    formatedSql += " as \"Column count\", ";
                                    formatedSql += "'";
                                    if(subscriptConnesction->data.tbldata.size() > 0)
                                        formatedSql += QVariant(subscriptConnesction->data.tbldata[0].size()).toString();
                                    else
                                        formatedSql += " 0 ";
                                    formatedSql += "'";
                                    formatedSql += " as \"Row count\" ";


                                    formatedSql += ", '";
                                    formatedSql += execTimeStr;
                                    formatedSql += "' ";
                                    formatedSql += "as \"Execution time\"";

                                }
                                else if(subCommandPatterns[i] == "SubexecAppendCSV")
                                {
                                    savefilecount++;
                                    if(!nodebug) qDebug() << "exporting (Append) to CSV " << saveName;

                                    if(subscriptConnesction->data.headers.size() > 0 && !subscriptConnesction->data.headers[0].startsWith("Error"))
                                        if(subscriptConnesction != nullptr && subscriptConnesction->data.tbldata.size() > 0)
                                            subscriptConnesction->data.AppendToCSV(QString(documentsDir + "/" +"CSV/") + QString(saveName) + QString(".csv"),csvDelimeter);

                                    if(subscriptConnesction->data.headers.size() > 0 && subscriptConnesction->data.headers[0] == "Error")
                                        formatedSql += " 'Error' as \"Status\", ";
                                    else
                                        formatedSql += " 'Success' as \"Status\", ";

                                    if(subscriptConnesction->data.headers.size() > 0 && subscriptConnesction->data.headers[0] == "Error")
                                    {
                                        formatedSql += "'";
                                        formatedSql += subscriptConnesction->data.tbldata[0][0];
                                        formatedSql += "'";
                                        formatedSql += " as \"Error message\", ";
                                    }
                                    else
                                    {
                                        formatedSql += "'";
                                        formatedSql += " ";
                                        formatedSql += "'";
                                        formatedSql += " as \"Error message\", ";
                                    }
                                    formatedSql += "'";
                                    formatedSql += QVariant(subscriptConnesction->data.tbldata.size()).toString();
                                    formatedSql += "'";
                                    formatedSql += " as \"Column count\", ";
                                    formatedSql += "'";
                                    if(subscriptConnesction->data.tbldata.size() > 0)
                                        formatedSql += QVariant(subscriptConnesction->data.tbldata[0].size()).toString();
                                    else
                                        formatedSql += " 0 ";
                                    formatedSql += "'";
                                    formatedSql += " as \"Row count\" ";


                                    formatedSql += ", '";
                                    formatedSql += execTimeStr;
                                    formatedSql += "' ";
                                    formatedSql += "as \"Execution time\"";

                                }
                                // script extensions/dblinks
                                else if(subCommandPatterns[i] == "SubexecToUnionAllTable")
                                {// exec into union all table
                                    // Oracle: select * from dual union all select * from dual
                                    // PostgreSQL/SQLite select * union all select *...
                                    savefilecount++;
                                    if(!nodebug) qDebug() <<"Reached SubexecToUnionAllTable implementation";

                                    if(subscriptConnesction != nullptr && subscriptConnesction->data.tbldata.size() > 0)
                                    {
                                        if(subscriptConnesction->data.tbldata[0].size() > 0)
                                        {
                                            formatedSql += "Select ";
                                            for(int t=0;t<subscriptConnesction->data.tbldata.size();t++)
                                            {
                                                formatedSql += "'";
                                                formatedSql += subscriptConnesction->data.tbldata[t][0];
                                                formatedSql += "'";
                                                formatedSql += " as \"";
                                                formatedSql += subscriptConnesction->data.headers[t];
                                                formatedSql += "\"";

                                                if(t + 1 <subscriptConnesction->data.tbldata.size()) // there will be next
                                                    formatedSql += ",";
                                            }
                                            if(oracle)
                                                formatedSql += "from dual";
                                            for(int j=1;j<subscriptConnesction->data.tbldata[0].size();j++)
                                            {
                                                formatedSql += " union all Select ";
                                                for(int t=0;t<subscriptConnesction->data.tbldata.size();t++)
                                                {
                                                    formatedSql += "'";
                                                    formatedSql += subscriptConnesction->data.tbldata[t][j];
                                                    formatedSql += "'";

                                                    if(t + 1 <subscriptConnesction->data.tbldata.size()) // there will be next
                                                        formatedSql += ",";
                                                }
                                                if(oracle)
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
                                            if(oracle)
                                                formatedSql += "from dual";
                                        }

                                    }


                                }
                                else if(subCommandPatterns[i] == "SubexecToMagic" || subCommandPatterns[i] == "ExcelToMagic" || subCommandPatterns[i] == "CSVToMagic")
                                {// exec into ('magic', 'element1') , ('magic', 'element2')
                                    //Probably oracle specific cuz oracle has 1k limit on 'in' arrays
                                    savefilecount++;
                                    if(!nodebug) qDebug() <<"Reached SubexecToMagic implementation";

                                    if(subscriptConnesction != nullptr && subscriptConnesction->data.tbldata.size() > 0)
                                    {
                                        for(int j=0;j<subscriptConnesction->data.tbldata[0].size();j++)
                                        {
                                            if(TableCutoffEnd>=0  && TableCutoffStart>=0 && j < TableCutoffStart)
                                                continue;
                                            if(TableCutoffEnd>=0 && TableCutoffStart>=0  && j > TableCutoffEnd)
                                                break;
                                            for(int t=0;t<subscriptConnesction->data.tbldata.size();t++)
                                            {
                                                if(t == neededMagicColumn || neededMagicColumn == -1)
                                                {
                                                    formatedSql += "('magic',";
                                                    formatedSql += "'";
                                                    formatedSql += subscriptConnesction->data.tbldata[t][j];
                                                    formatedSql += "'";
                                                    formatedSql += ")";
                                                    if(((TableCutoffEnd>=0 && TableCutoffStart>=0  && j+1 <= TableCutoffEnd && j + 1 <subscriptConnesction->data.tbldata[0].size()) || (TableCutoffEnd<0 && TableCutoffStart<0 && j + 1 <subscriptConnesction->data.tbldata[0].size())) || (t + 1 <subscriptConnesction->data.tbldata.size() && neededMagicColumn == -1)) // there will be next
                                                        formatedSql += ",";
                                                }
                                            }
                                        }
                                    }
                                }
                                else if(subCommandPatterns[i] == "SubexecToArray" || subCommandPatterns[i] == "ExcelToArray" || subCommandPatterns[i] == "CSVToArray")
                                {// exec into 'element1','element2','element3'
                                    savefilecount++;
                                    if(!nodebug) qDebug() <<"Reached SubexecToArray implementation";
                                    if(subscriptConnesction != nullptr && subscriptConnesction->data.tbldata.size() > 0)
                                    {
                                        for(int j=0;j<subscriptConnesction->data.tbldata[0].size();j++)
                                        {
                                            if(TableCutoffEnd>=0  && TableCutoffStart>=0 && j < TableCutoffStart)
                                                continue;
                                            if(TableCutoffEnd>=0 && TableCutoffStart>=0  && j > TableCutoffEnd)
                                                break;
                                            for(int t=0;t<subscriptConnesction->data.tbldata.size();t++)
                                            {
                                                if(t == neededMagicColumn || neededMagicColumn == -1)
                                                {
                                                    formatedSql += "'";
                                                    formatedSql += subscriptConnesction->data.tbldata[t][j];
                                                    formatedSql += "' ";
                                                    if(((TableCutoffEnd>=0 && TableCutoffStart>=0  && j+1 <= TableCutoffEnd && j + 1 <subscriptConnesction->data.tbldata[0].size()) || (TableCutoffEnd<0 && TableCutoffStart<0 && j + 1 <subscriptConnesction->data.tbldata[0].size())) || (t + 1 <subscriptConnesction->data.tbldata.size() && neededMagicColumn == -1)) // there will be next
                                                        formatedSql += ",";
                                                }
                                            }
                                        }
                                    }




                                }
                                else if(subCommandPatterns[i] == "ExcelToSqliteTable"|| subCommandPatterns[i] == "CSVToSqliteTable" || subCommandPatterns[i] == "ExcelToLocalDBTable"|| subCommandPatterns[i] == "CSVToLocalDBTable")
                                {
                                    qDebug() << "subscriptDBname (exceltosqlitetable local table name): " << subscriptDBname;
                                    if(subscriptConnesction->data.headers.size() > 0)
                                    {
                                        if(subscriptConnesction != nullptr && subscriptConnesction->data.tbldata.size() > 0)
                                        {
                                            subscriptConnesction->data.ExportToSQLiteTable(subscriptDBname);
                                        }
                                        else
                                            qDebug() << "subscriptConnesction != nullptr && subscriptConnesction->data.tbldata.size() > 0 Failed";
                                    }
                                    else
                                        qDebug() << "subscriptConnesction->data.headers.size() > 0 Failed";
                                }

                                if(!nodebug) qDebug() << "closing subconnection";
                                subscriptConnesction->db.close();
                                delete subscriptConnesction;
                                subscriptConnesction = nullptr;
                                if(!nodebug) qDebug() << "closed";


                                if(stopNow)
                                {
                                    stopNow=false;
                                    executionTime = executionEnd.toSecsSinceEpoch() - executionStart.toSecsSinceEpoch();
                                    executionEnd = QDateTime::currentDateTime();
                                    if(!nodebug) qDebug() << "Execd";
                                    executing = false;
                                    queryExecutionState = 4;
                                    emit execedSql();
                                    return false;
                                }
                            }

                            if(subCommandPatterns[i].startsWith("SendMail") )
                            {
                                QString host;
                                QString Sender;
                                QString SenderName;
                                QStringList to;
                                QStringList cc;
                                QString Subject;
                                QString messageText;
                                QStringList attachments;


                                int varcount = 0;
                                for(auto s : funcVariables)
                                {
                                    if(varcount ==0)
                                        host = s;
                                    if(varcount ==1)
                                        Sender = s;
                                    if(varcount ==2)
                                        SenderName = s;
                                    if(varcount ==3)
                                        to = s.split(',');
                                    if(varcount ==4)
                                        cc = s.split(',');
                                    if(varcount ==5)
                                        Subject = s;
                                    if(varcount ==6)
                                        messageText = s;
                                    if(varcount ==7)
                                        attachments = s.split(',');
                                    varcount ++;
                                }

                                if(!nodebug) qDebug() << "sending :";
                                if(!nodebug) qDebug() << "host = " << host;
                                if(!nodebug) qDebug() << "Sender = " << Sender;
                                if(!nodebug) qDebug() << "SenderName = " << SenderName;
                                if(!nodebug) qDebug() << "to = " << to;
                                if(!nodebug) qDebug() << "cc = " << cc;
                                if(!nodebug) qDebug() << "Subject = " << Subject;
                                if(!nodebug) qDebug() << "messageText = " << messageText;
                                if(!nodebug) qDebug() << "attachments = " << attachments;

                                emit sendMail(host,Sender,SenderName,to,cc,Subject,messageText,attachments);


                            }

                            // Async execution.
                            //// Start async - start launching each subexec on separate threads.
                            //// Wait async - wait all async executions, paste all temporary placeholders(_DBL_AsyncExecutionTempValue) with results

                            // subCommandPatterns.push_back("StartAsyncExecution");
                            // subCommandPatterns.push_back("AwaitAsyncExecution");

                            if(subCommandPatterns[i].trimmed() == "StartAsyncExecution")
                            {
                                asyncExecution_recordExecs = true;
                                asyncExecution_pasteExecsData = false;

                            }
                            if(subCommandPatterns[i].trimmed() == "AwaitAsyncExecution")
                            {
                                asyncExecution_recordExecs = false;
                                asyncExecution_pasteExecsData = false;



                                for(auto t : asyncExecution_threads)
                                {
                                    qDebug() << "wait thread " << t;
                                    if(t->isRunning())
                                        t->wait();
                                    qDebug() << "thread done" << t;
                                }
                                asyncExecution_threads.clear();
                                qDebug() << "threads done and cleared";
                                reset = true;
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
            sql = formatedSql;
            if(!nodebug) qDebug()<< "";
            if(!nodebug) qDebug()<< "";
            if(!nodebug) qDebug()<< "";
            if(!nodebug) qDebug()<< "loopcount " << loopcount << " sql: "<< sql;
            if(!nodebug) qDebug()<< "";
            if(!nodebug) qDebug()<< "";
            if(!nodebug) qDebug()<< "";
            if(loopcount > 5) break;
        }
        if(!nodebug) qDebug()<<"Exiting sql subcomand processing........";
        if(!nodebug) qDebug()<<"Resulting formatedSql is";
        str = formatedSql.trimmed();

    }
    executing = true;
    // run result
    while(str.endsWith('\n') || str.endsWith('}') ||str.endsWith('\t') || str.endsWith(' '))
        str.resize(str.size()-1);
    if(!nodebug) qDebug()<<str;
    if(!nodebug) qDebug() << "creating query";
    emit queryBeginCreating();

    data.typecount.clear();

    queryExecutionState = 1;
    Last_sqlCode = str;

    data.sqlCode = str;
    data.LastDatabase = this->dbname;
    data.LastDriver = this->driver;
    data.LastUser = this->usrname;

    if(customSQlite && sqlite3DBConnectionIsOpen && sqlite3DBConnection != nullptr)
    {
        sqlite3* db = (sqlite3*) sqlite3DBConnection;
        sqlite3_stmt *stmt;
        std::string stdstr = str.toStdString();
        const char *sql = stdstr.c_str();

        emit queryBeginExecuting();
        queryExecutionState = 2;
        int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
        if (rc != SQLITE_OK)
            if(!nodebug) qDebug() << "Failed to prepare statement: " << sqlite3_errmsg(db);



        emit querySuccess();
        queryExecutionState = 3;


        data.headers.clear();
        for(int i=0; i < sqlite3_column_count(stmt) ; i++)
        {
            data.headers << sqlite3_column_name(stmt,i);
        }

        data.tbldata.clear();
        data.tbldata.resize(data.headers.size());
        data.typecount.clear();
        data.typecount.resize(data.tbldata.size());


        dataDownloading = true;
        int rowcount = 0;
        while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
        {
            for(int i=0;i < data.headers.size();i++)
            {

                QString str = QString().fromUtf8((const char*)sqlite3_column_text(stmt, i));

                data.tbldata[i].push_back(fixQStringType(str));//

                if(data.tbldata[i].back().size()>0)
                {
                    while(data.typecount[i].size() <= fixQStringType_lasttype)
                        data.typecount[i].emplace_back();
                    data.typecount[i][fixQStringType_lasttype]++;
                }

            }
            rowcount++;
            if(stopAt500Lines && rowcount > 500)
                break;
            if(stopNow)
            {
                stopNow=false;
                break;
            }
        }


        data.maxVarTypes.clear();
        data.maxVarTypes.resize( data.typecount.size());

        for(int i=0;i < data.typecount.size();i++)
        {
            int maxvt = 0;
            for(int a  = 0; a < data.typecount[i].size();a++)
            {
                if(a == 0 || maxvt <= data.typecount[i][a])
                {
                    data.maxVarTypes[i] = a;
                    maxvt = data.typecount[i][a];
                }

            }
        }
        // for(int i=0;i < data.typecount.size();i++)
        // {
        //     if(data.typecount[i].size()>10)
        //     {
        //         if(data.typecount[i][10] > 0)
        //             data.maxVarTypes[i] = 10;

        //     }
        // }

        dataDownloading = false;

        if (rc != SQLITE_DONE && sqlite3_errmsg(db) != "another row available")
        {
            if(!nodebug) qDebug() << "Error executing query: " << sqlite3_errmsg(db);
            data.headers.clear();
            data.headers.push_back("Error");

            data.tbldata.clear();
            data.tbldata.resize(data.headers.size());
            data.typecount.clear();
            data.typecount.resize(data.tbldata.size());

            data.tbldata[0].push_back(sqlite3_errmsg(db));

            lastErrorPos = sqlite3_error_offset(db);
            if(!nodebug) qDebug() << "stopped";
            OCI_lastenv = nullptr;
            OCI_lastcon = nullptr;
            executing = false;
            executionEnd = QDateTime::currentDateTime();
            executionTime = executionEnd.toSecsSinceEpoch() - executionStart.toSecsSinceEpoch();
            lastLaunchIsError = true;
            emit execedSql();
            queryExecutionState = 4;
            return false;

        }
        sqlite3_finalize(stmt);
        executing = false;
        dataDownloading = false;
        queryExecutionState = 4;
        executionEnd = QDateTime::currentDateTime();
        executionTime = executionEnd.toSecsSinceEpoch() - executionStart.toSecsSinceEpoch();
        emit execedSql();
        return true;
    }



    if(customPSQL && pg_connected && ptr_PGconn != nullptr)
    {
        if(!nodebug) qDebug() << "in postgres";
        std::string stdstr = str.toUtf8().toStdString();
        const char *sql = stdstr.c_str();



        emit queryBeginExecuting();
        queryExecutionState = 2;
        if(!nodebug) qDebug() << "PQexec";
        PGresult* res = PQexec((PGconn*) ptr_PGconn, sql);

        if(!nodebug) qDebug() << "PQexec checks";
        if (PQresultStatus(res) != PGRES_TUPLES_OK && PQresultStatus(res) != PGRES_COMMAND_OK) {


            data.headers.clear();

            data.headers << "Error";
            data.headers << "Error position";

            data.tbldata.clear();
            data.tbldata.resize(data.headers.size());
            data.tbldata[0].push_back(QString().fromUtf8(PQresultErrorMessage(res)));
            data.tbldata[0].push_back(QString().fromUtf8(PQerrorMessage((PGconn*) ptr_PGconn)));
            data.tbldata[1].push_back(QString().fromUtf8(PQresultErrorField(res,PG_DIAG_STATEMENT_POSITION)));
            data.tbldata[1].push_back(QString().fromUtf8(PQresultVerboseErrorMessage(res,PGVerbosity::PQERRORS_VERBOSE,PGContextVisibility::PQSHOW_CONTEXT_ERRORS)));

            lastLaunchIsError = true;
            lastErrorPos = QVariant(QString().fromUtf8(PQresultErrorField(res,PG_DIAG_STATEMENT_POSITION))).toInt();


            data.typecount.clear();
            dataDownloading = false;

            PQclear(res);
            executing = false;
            dataDownloading = false;
            queryExecutionState = 4;
            executionEnd = QDateTime::currentDateTime();
            executionTime = executionEnd.toSecsSinceEpoch() - executionStart.toSecsSinceEpoch();
            emit execedSql();
            return false;

        }
        else if(PQresultStatus(res) == PGRES_COMMAND_OK)
        {

            PQclear(res);
            data.headers.clear();
            data.tbldata.clear();
            data.typecount.clear();
            dataDownloading = false;

            executing = false;
            dataDownloading = false;
            queryExecutionState = 4;
            executionEnd = QDateTime::currentDateTime();
            executionTime = executionEnd.toSecsSinceEpoch() - executionStart.toSecsSinceEpoch();
            emit execedSql();
            return true;
        }
        emit querySuccess();
        queryExecutionState = 3;

        int rows = PQntuples(res);
        int cols = PQnfields(res);

        data.saveRowSize = 0;
        data.saveRowsDone = 0;
        if(!nodebug) qDebug() << rows << cols << "rows << cols";

        if(cols <=0)
        {

            data.headers.clear();

            data.headers << "Error";

            data.tbldata.clear();
            data.tbldata.resize(data.headers.size());
            data.tbldata[0].push_back(QString().fromUtf8(PQresultErrorMessage(res)));
            data.tbldata[0].push_back(QString().fromUtf8(PQerrorMessage((PGconn*) ptr_PGconn)));

            data.typecount.clear();

            dataDownloading = false;

            PQclear(res);
            executing = false;
            dataDownloading = false;
            queryExecutionState = 4;
            executionEnd = QDateTime::currentDateTime();
            executionTime = executionEnd.toSecsSinceEpoch() - executionStart.toSecsSinceEpoch();
            emit execedSql();
            return false;
        }

        data.headers.clear();

        for (int j = 0; j < cols; ++j)
        {
            data.headers << QString().fromUtf8(PQfname(res,j));

        }
        data.tbldata.clear();
        data.tbldata.resize(data.headers.size());
        data.typecount.clear();
        data.typecount.resize(data.tbldata.size());
        dataDownloading = true;

        for (int j = 0; j < cols; ++j)
        {
            data.tbldata[j].reserve(rows);
        }


        data.saveRowSize = rows;
        for (int j = 0; j < rows; j++)
        {

            if(stopAt500Lines && j > 500)
                break;
            if(stopNow)
            {
                stopNow=false;
                break;
            }
            for (int i = 0; i < cols; i++)
            {


                data.tbldata[i].emplaceBack(fixQStringType(QString().fromUtf8(PQgetvalue(res, j, i))));


                while(data.typecount[i].size() <= fixQStringType_lasttype)
                    data.typecount[i].emplace_back();

                data.typecount[i][fixQStringType_lasttype]++;

            }
            data.saveRowsDone++;

        }
        dataDownloading = false;



        data.maxVarTypes.clear();
        data.maxVarTypes.resize( data.typecount.size());

        for(int i=0;i < data.typecount.size();i++)
        {
            int maxvt = 0;
            for(int a  = 0; a < data.typecount[i].size();a++)
            {
                if(a == 0 || maxvt <= data.typecount[i][a])
                {
                    data.maxVarTypes[i] = a;
                    maxvt = data.typecount[i][a];
                }

            }
        }
        // for(int i=0;i < data.typecount.size();i++)
        // {
        //     if(data.typecount[i].size()>10)
        //     {
        //         if(data.typecount[i][10] > 0)
        //             data.maxVarTypes[i] = 10;

        //     }
        // }

        dataDownloading = false;
        PQclear(res);


        executing = false;
        dataDownloading = false;
        queryExecutionState = 4;
        executionEnd = QDateTime::currentDateTime();
        executionTime = executionEnd.toSecsSinceEpoch() - executionStart.toSecsSinceEpoch();
        emit execedSql();
        return true;
    }


#ifdef Oracle_OCI_Driver
    if(customOracle)
    {
        try{

        try{

            OCI_lastenv = oracle::occi::Environment::createEnvironment ( "CL8MSWIN1251", "CL8MSWIN1251",oracle::occi::Environment::Mode::THREADED_MUTEXED);//oracle::occi::Environment::DEFAULT);

            QString connection_string ="(DESCRIPTION=(ADDRESS=(PROTOCOL=tcp) (HOST=" + ipAddres.trimmed() + ") (PORT=" + port.trimmed() + "))(CONNECT_DATA=(SERVICE_NAME=" + schemaName.trimmed() + ")))";
            OCI_lastcon = ((oracle::occi::Environment*)OCI_lastenv)->createConnection (usrname.trimmed().toStdString(),password.trimmed().toStdString(),connection_string.toStdString());

            emit queryBeginExecuting();
            queryExecutionState = 2;

            bool status = true;
            try {
                OCI_laststmt = nullptr;
                OCI_laststmt = ((oracle::occi::Connection*)OCI_lastcon)->createStatement (str.toLocal8Bit().constData());

                oracle::occi::ResultSet *rset = nullptr;
                rset = ((oracle::occi::Statement*)OCI_laststmt)->executeQuery ();
                ((oracle::occi::Statement*)OCI_laststmt)->setPrefetchRowCount(5000); // enable bulk collect
                emit querySuccess();
                queryExecutionState = 3;
                dataDownloading = true;

                std::vector<int> types;
                int type = 0;

                int columnCount = 0;
                if(true)
                {
                    // std::vector<oracle::occi::MetaData> mtl = rset->getColumnListMetaData();

                    static OCIError *errhp = NULL;
                    (void) OCIHandleAlloc( (dvoid *) ((oracle::occi::Environment*)OCI_lastenv)->getOCIEnvironment(), (dvoid **) &errhp, OCI_HTYPE_ERROR, (size_t) 0, (dvoid **) 0);




                    OCIParam     *mypard = (OCIParam *) 0;
                    ub2          dtype =0;
                    text         *col_name = 0;
                    ub4          counter, col_name_len, char_semantics;

                    counter = 1;
                    int parm_status = OCIParamGet(((oracle::occi::Statement*)OCI_laststmt)->getOCIStatement(), OCI_HTYPE_STMT, errhp,
                                              (void **)&mypard, (ub4) counter);

                    /* Loop only if a descriptor was successfully retrieved for
                       current position, starting at 1 */

                    data.headers.clear();
                    data.typecount.clear();
                    columnCount = 0;
                    while (parm_status == OCI_SUCCESS) {
                        /* Retrieve the data type attribute */
                        columnCount++;

                        OCIAttrGet((void*) mypard, (ub4) OCI_DTYPE_PARAM,
                                                   (void*) &dtype,(ub4 *) 0, (ub4) OCI_ATTR_DATA_TYPE,
                                                   (OCIError *) errhp  );

                        /* Retrieve the column name attribute */
                        col_name_len = 0;
                        text* nulltxt = 0;
                        col_name = nulltxt;
                        OCIAttrGet((void*) mypard, (ub4) OCI_DTYPE_PARAM,
                                                   (void**) &col_name, (ub4 *) &col_name_len, (ub4) OCI_ATTR_NAME,
                                                   (OCIError *) errhp );

                        /* Retrieve the length semantics for the column */
                        char_semantics = 0;
                        OCIAttrGet((void*) mypard, (ub4) OCI_DTYPE_PARAM,
                                                   (void*) &char_semantics,(ub4 *) 0, (ub4) OCI_ATTR_CHAR_USED,
                                                   (OCIError *) errhp  );
                        int col_width = 0;
                        if (char_semantics)
                            /* Retrieve the column width in characters */
                            OCIAttrGet((void*) mypard, (ub4) OCI_DTYPE_PARAM,
                                                       (void*) &col_width, (ub4 *) 0, (ub4) OCI_ATTR_CHAR_SIZE,
                                                       (OCIError *) errhp  );
                        else
                            /* Retrieve the column width in bytes */
                            OCIAttrGet((void*) mypard, (ub4) OCI_DTYPE_PARAM,
                                                       (void*) &col_width,(ub4 *) 0, (ub4) OCI_ATTR_DATA_SIZE,
                                                       (OCIError *) errhp  );

                        types.push_back(0);
                        data.headers.push_back(0);
                        types.back() = detectType(dtype);
                        qDebug() << QString().fromLocal8Bit((const char*)col_name,col_name_len) << " type = " <<dtype << types.back();
                        data.headers.back() = QString().fromLocal8Bit((const char*)col_name,col_name_len);


                        data.typecount.emplace_back();
                        while(data.typecount.back().size() <= types.back())
                            data.typecount.back().emplace_back();

                        data.typecount.back()[types.back()]++;


                        /* increment counter and get next descriptor, if there is one */
                        counter++;
                        parm_status = OCIParamGet((void *)((oracle::occi::Statement*)OCI_laststmt)->getOCIStatement(), OCI_HTYPE_STMT, errhp,
                                                  (void **)&mypard, (ub4) counter);
                    } /* while */


                    if(!nodebug) qDebug() << columnCount;
                    data.tbldata.clear();
                    data.tbldata.resize(columnCount);
                    data.maxVarTypes.clear();
                    data.maxVarTypes.resize( columnCount);

                    try
                    {
                        OCIHandleFree(errhp,OCI_HTYPE_ERROR);
                        //con->terminateStatement(stmt);

                        //env->terminateConnection(con);
                    }
                    catch(oracle::occi::SQLException ex)
                    {
                        qDebug() << ex.getErrorCode() << ex.getMessage();
                    }
                }

                for(int i=0;i < data.typecount.size();i++)
                {
                    int maxvt = 0;
                    for(int a  = 0; a < data.typecount[i].size();a++)
                    {
                        if(a == 0 || maxvt <= data.typecount[i][a])
                        {
                            data.maxVarTypes[i] = a;
                            maxvt = data.typecount[i][a];
                        }

                    }
                }
                // for(int i=0;i < data.typecount.size();i++)
                // {
                //     if(data.typecount[i].size()>10)
                //     {
                //         if(data.typecount[i][10] > 0)
                //             data.maxVarTypes[i] = 10;

                //     }
                // }
                int ccnt = 0;



                while (rset->next() && columnCount > 0)
                {
                    ccnt ++;
                    for(int i=0;i < columnCount; i ++)
                    {

                        if(types[i]!=16)
                        {

                            if(types[i]!=6)
                                data.tbldata[i].push_back(QString().fromLocal8Bit(rset->getString(i+1)));
                            else
                            {
                                QString tmpstr = QString().fromLocal8Bit(rset->getString(i+1)).replace(',','.');


                                data.tbldata[i].push_back(tmpstr);


                            }
                        }
                        else if(!rset->isNull(i + 1))
                        {
                            data.tbldata[i].push_back(fromOCIDateTimeToString(rset->getDate(i+1)));
                        }
                        else
                        {
                            data.tbldata[i].push_back("");
                        }



                    }

                    if(stopAt500Lines && ccnt > 500)
                        break;
                    if(stopNow)
                    {
                        stopNow=false;
                        break;
                    }
                }
                //mtl.pop_back();

                for(auto x : data.maxVarTypes)
                    qDebug() << x;

                qDebug() << "data.maxVarTypes == data.tbldata.size()" << (data.maxVarTypes.size() == data.tbldata.size() );

                dataDownloading = false;
                if(rset!=nullptr)
                    rset->cancel();
                if(((oracle::occi::Statement*)OCI_laststmt)!=nullptr)
                    ((oracle::occi::Statement*)OCI_laststmt)->closeResultSet (rset);
            }catch(oracle::occi::SQLException ex)
            {
                lastLaunchIsError = true;
                if(!nodebug) qDebug()<<"oracle::occi::SQLException at runSelect" << " Error number: "<<  ex.getErrorCode()  << "    "<<ex.getMessage() ;
                data.headers.clear();
                data.tbldata.clear();
                data.tbldata.emplace_back();
                data.headers.push_back("Error");
                data.headers.push_back("db Error");
                data.headers.push_back("driver Error");


                int errorpos = 0; //
                status = false;
                lastOracleError = ex.getMessage().c_str();
                if(((oracle::occi::Statement*)OCI_laststmt)!= nullptr && ((oracle::occi::Environment*)OCI_lastenv) != nullptr)
                {
                    try
                    {
                        static OCIError *errhp = NULL;
                        if(!nodebug) qDebug() << "generating error";
                        (void) OCIHandleAlloc( (dvoid *) ((oracle::occi::Environment*)OCI_lastenv)->getOCIEnvironment(), (dvoid **) &errhp, OCI_HTYPE_ERROR, (size_t) 0, (dvoid **) 0);



                        if(!nodebug) qDebug() << "retrieving error";
                        OCIAttrGet((dvoid *)((oracle::occi::Statement*)OCI_laststmt)->getOCIStatement(), (ub4)OCI_HTYPE_STMT,
                                   (dvoid *)&errorpos, (ub4 *)NULL,
                                   (ub4)OCI_ATTR_PARSE_ERROR_OFFSET, errhp);

                        if(!nodebug) qDebug() << "lastErrorPos = errorpos";
                        lastErrorPos = errorpos;
                        if(!nodebug) qDebug() <<"errpos" << errorpos;

                        OCIHandleFree(errhp,OCI_HTYPE_ERROR);
                    }
                    catch(oracle::occi::SQLException ex)
                    {
                        if(!nodebug) qDebug() << ex.getErrorCode() << ex.getMessage();
                    }
                }
                if(stopNow)
                {

                    try {
                        if(((oracle::occi::Statement*)OCI_laststmt)!= nullptr)
                            ((oracle::occi::Connection*)OCI_lastcon)->terminateStatement (((oracle::occi::Statement*)OCI_laststmt));
                        OCI_laststmt = nullptr;

                        if(((oracle::occi::Environment*)OCI_lastenv)!= nullptr && ((oracle::occi::Connection*)OCI_lastcon) != nullptr)
                            ((oracle::occi::Environment*)OCI_lastenv)->terminateConnection (((oracle::occi::Connection*)OCI_lastcon));

                        if(((oracle::occi::Environment*)OCI_lastenv)!= nullptr)
                            oracle::occi::Environment::terminateEnvironment (((oracle::occi::Environment*)OCI_lastenv));
                    }
                    catch(oracle::occi::SQLException ex)
                    {

                        if(!nodebug) qDebug()<<"oracle::occi::SQLException at runSelect" << " Error number: "<<  ex.getErrorCode()  << "    "<<ex.getMessage() ;
                    }

                    if(!dataDownloading)
                    {
                        data.headers.clear();
                        data.tbldata.clear();
                        data.tbldata.emplace_back();
                        data.headers.push_back("Error");
                        data.tbldata.back().emplace_back("user canceled query");
                    }
                    if(!nodebug) qDebug() << "stopped";
                    OCI_lastenv = nullptr;
                    OCI_lastcon = nullptr;
                    executing = false;
                    executionEnd = QDateTime::currentDateTime();
                    executionTime = executionEnd.toSecsSinceEpoch() - executionStart.toSecsSinceEpoch();
                    lastLaunchIsError = true;
                    emit execedSql();
                    queryExecutionState = 4;
                    return false;
                }
                dataDownloading = false;
                lastLaunchIsError = true;


            }

            if(!nodebug) qDebug() << "closing statement";
            if(((oracle::occi::Statement*)OCI_laststmt)!= nullptr)
                ((oracle::occi::Connection*)OCI_lastcon)->terminateStatement (((oracle::occi::Statement*)OCI_laststmt));
            OCI_laststmt = nullptr;

            if(!nodebug) qDebug() << "closing connection";
            if(((oracle::occi::Environment*)OCI_lastenv)!= nullptr && ((oracle::occi::Connection*)OCI_lastcon) != nullptr)
                ((oracle::occi::Environment*)OCI_lastenv)->terminateConnection (((oracle::occi::Connection*)OCI_lastcon));

            if(!nodebug) qDebug() << "closing env";
            if(((oracle::occi::Environment*)OCI_lastenv)!= nullptr)
                oracle::occi::Environment::terminateEnvironment (((oracle::occi::Environment*)OCI_lastenv));

            //currentRunningOracleDriver = nullptr;
            if(!status)
            {
                executing = false;
                data.headers.clear();
                data.tbldata.clear();
                data.tbldata.emplace_back();
                data.headers.push_back("Error");
                data.headers.push_back("db Error");
                data.headers.push_back("driver Error");

                QString errStr = lastOracleError;
                if(errStr.contains(" line "))
                {
                    QString searchstr = " line ";
                    while(errStr.contains(searchstr))
                    {

                        QString replaceThe = "";
                        int match = 0;
                        int i =0;
                        while(i < errStr.size() && match < searchstr.size())
                        {
                            if(searchstr[match] == errStr[i])
                            {
                                replaceThe+=errStr[i];
                                match++;
                            }
                            else
                            {
                                match = 0;
                                replaceThe.clear();
                            }
                            i++;
                        }
                        QString linenum = "";
                        while(i < errStr.size() && errStr[i].isDigit())
                            linenum+= errStr[i++];
                        if(linenum.size()>0)
                        {
                            errStr =        errStr.replace(searchstr + linenum," code_pos: "+QVariant(QVariant(linenum).toInt() + _code_start_line).toString());

                            if(!nodebug) qDebug()<<"line num is "<< linenum ;
                        }
                    }
                    data.tbldata.back().push_back(errStr);
                }
                data.tbldata.back().push_back(lastOracleError);
                data.tbldata.back().push_back(errStr);

                data.tbldata.emplace_back();
                data.tbldata.back().push_back(QString("Error position ") + QVariant(lastErrorPos + _code_start_pos).toString());


                if(!nodebug) qDebug() << "Execd";
                OCI_lastenv = nullptr;
                OCI_lastcon = nullptr;
                executing = false;
                executionEnd = QDateTime::currentDateTime();
                executionTime = executionEnd.toSecsSinceEpoch() - executionStart.toSecsSinceEpoch();
                lastLaunchIsError = true;
                emit execedSql();
                queryExecutionState = 4;
                return false;
            }
            OCI_lastenv = nullptr;
            OCI_lastcon = nullptr;
            executing = false;
            executionEnd = QDateTime::currentDateTime();
            executionTime = executionEnd.toSecsSinceEpoch() - executionStart.toSecsSinceEpoch();
            queryExecutionState = 4;
            emit execedSql();
            return true;
        }
        catch (oracle::occi::SQLException ex){
            if(!nodebug) qDebug() << "oracledb error";
            lastLaunchIsError = true;
            lastErrorPos = -1;
            if(!nodebug) qDebug() << "error creating connection" << ex.getErrorCode() << ex.getMessage();
            executing = false;
            data.headers.clear();
            data.tbldata.clear();
            data.tbldata.emplace_back();
            data.headers.push_back("Error");
            data.headers.push_back("db Error");
            data.headers.push_back("driver Error");

            QString errStr = ex.getMessage().c_str();
            if(errStr.contains(" line "))
            {
                QString searchstr = " line ";
                while(errStr.contains(searchstr))
                {
                    QString replaceThe = "";
                    int match = 0;
                    int i =0;
                    while(i < errStr.size() && match < searchstr.size())
                    {
                        if(searchstr[match] == errStr[i])
                        {
                            replaceThe+=errStr[i];
                            match++;
                        }
                        else
                        {
                            match = 0;
                            replaceThe.clear();
                        }
                        i++;
                    }
                    QString linenum = "";
                    while(i < errStr.size() && errStr[i].isDigit())
                        linenum+= errStr[i++];
                    if(linenum.size()>0)
                    {
                        errStr = errStr.replace(searchstr + linenum," code_pos: "+QVariant(QVariant(linenum).toInt() + _code_start_line).toString());

                        if(!nodebug) qDebug()<<"line num is "<< linenum ;
                    }
                }
                data.tbldata.back().push_back(errStr);
            }
            data.tbldata.back().push_back(ex.getMessage().c_str());
            data.tbldata.back().push_back(errStr);

            executionTime = executionEnd.toSecsSinceEpoch() - executionStart.toSecsSinceEpoch();
            //currentRunningOracleDriver = nullptr;

            if(!nodebug) qDebug() << "Execd";
            OCI_lastenv = nullptr;
            OCI_lastcon = nullptr;
            executing = false;
            executionEnd = QDateTime::currentDateTime();
            executionTime = executionEnd.toSecsSinceEpoch() - executionStart.toSecsSinceEpoch();
            emit execedSql();
            queryExecutionState = 4;
            return false;
        }



        }
        catch (...)//  i looove oracle
        {
            qDebug() << "unknown error when trying to create error for oracle db";
            QMessageBox mb;
            mb.setText("Unknown error when trying to create error for oracle db. Change db");
            mb.exec();
            //lasterror = "unknown error when trying to create error for oracle db";
        }
        if(!nodebug) qDebug() << "oracledb execed";

        OCI_lastenv = nullptr;
        OCI_lastcon = nullptr;
        //currentRunningOracleDriver = nullptr;
        executionEnd = QDateTime::currentDateTime();
        executionTime = executionEnd.toSecsSinceEpoch() - executionStart.toSecsSinceEpoch();
        emit execedSql();
        executing = false;
        queryExecutionState = 4;
        return true;
    }
#endif

    OCI_lastenv = nullptr;
    OCI_lastcon = nullptr;
    //currentRunningOracleDriver = nullptr;
    QSqlQuery q(db);
    query = &q;
    if(!nodebug) qDebug() << "created query";

    q.setForwardOnly(true);
    data.headers.clear();
    if(!nodebug) qDebug() << "executing query";
    emit queryBeginExecuting();
    queryExecutionState = 2;
    qDebug() << q.driver()->hasFeature(QSqlDriver::CancelQuery) << " << QSqlDriver::CancelQuery";
    if(q.exec(str))
    {
        if(!nodebug) qDebug() << "query sucess.  isSelect = " << q.isSelect();
        emit querySuccess();
        queryExecutionState = 3;
        for(int a=0,total = q.record().count(); a<total;a++)
            data.headers << q.record().fieldName(a);
        int i=0;
        data.tbldata.clear();
        data.tbldata.resize(data.headers.size());
        data.typecount.clear();
        data.typecount.resize(data.tbldata.size());
        while(q.next())
        {
            for(int a=0,total = q.record().count(); a<total;a++)
            {

                QVariant var = fixQVariantTypeFormat(q.value(a));
                //qDebug() << q.value(a) << " -> " << var;

                while(data.typecount[a].size() <= var.typeId())
                    data.typecount[a].emplace_back();
                data.typecount[a][var.typeId()]++;

                data.tbldata[a].push_back(var.toString());
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

        data.maxVarTypes.clear();
        data.maxVarTypes.resize( data.typecount.size());

        for(int i=0;i < data.typecount.size();i++)
        {
            int maxvt = 0;
            for(int a  = 0; a < data.typecount[i].size();a++)
            {
                if(a == 0 || maxvt <= data.typecount[i][a])
                {
                    data.maxVarTypes[i] = a;
                    maxvt = data.typecount[i][a];
                }
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

        // if oracle, run oracle specific error handling
        if(oracle)
        {
            QString GetErrorSQL = "declare    v_count number;    v_bad_sql varchar2(32767) :=         '";
            while(str.endsWith(' ') || str.endsWith('\n')|| str.endsWith('\t')|| str.endsWith(';'))
                str.resize(str.size()-1);
            GetErrorSQL +=  str.replace('\'', "''" );
            GetErrorSQL +="';begin    execute immediate v_bad_sql into v_count;exception when others then    begin        execute immediate            'begin for i in ( '||v_bad_sql||') loop null; end loop; end;';    exception when others then        dbms_output.put_line(sqlerrm);  RAISE; end;  end;";
            if(q.exec(GetErrorSQL))
            {
                while(q.next())
                {
                    data.tbldata.back().push_back(q.value(0).toString());
                    data.tbldata.back().push_back(q.value(1).toString());
                    data.tbldata.back().push_back(q.value(2).toString());
                }
            }
            if(q.lastError().text().contains(" line "))
            {
                QString errStr = q.lastError().text();
                QString searchstr = " line ";
                while(errStr.contains(searchstr))
                {
                    QString replaceThe = "";
                    int match = 0;
                    int i =0;
                    while(i < errStr.size() && match < searchstr.size())
                    {
                        if(searchstr[match] == errStr[i])
                        {
                            replaceThe+=errStr[i];
                            match++;
                        }
                        else
                        {
                            match = 0;
                            replaceThe.clear();
                        }
                        i++;
                    }
                    QString linenum = "";
                    while(i < errStr.size() && errStr[i].isDigit())
                        linenum+= errStr[i++];
                    if(linenum.size()>0)
                    {
                        errStr =        errStr.replace(searchstr + linenum," code_pos: "+QVariant(QVariant(linenum).toInt() + _code_start_line).toString());

                        if(!nodebug) qDebug()<<"line num is "<< linenum ;
                    }
                }
                data.tbldata.back().push_back(errStr);
            }
            data.tbldata.back().push_back(q.lastError().text().toStdString().c_str());
            data.tbldata.back().push_back(q.lastError().databaseText());
            data.tbldata.back().push_back(q.lastError().driverText());
        }
        else
        {
            if(!nodebug) qDebug() << "Error in sql query:"<< q.lastError().text().toStdString().c_str();
            if(!nodebug) qDebug() << "Error from database:"<< tr(q.lastError().databaseText().toStdString().c_str());
            if(!nodebug) qDebug() << "Error from driver:"<< q.lastError().driverText();
            if(!nodebug) qDebug() << sql;
            data.tbldata.back().push_back(q.lastError().text().toStdString().c_str());
            data.tbldata.back().push_back(q.lastError().databaseText());
            data.tbldata.back().push_back(q.lastError().driverText());
        }

        q.clear();
        q.finish();
        query = nullptr;

        executionEnd = QDateTime::currentDateTime();
        executionTime = executionEnd.toSecsSinceEpoch() - executionStart.toSecsSinceEpoch();
        if(!nodebug) qDebug() << "Execd";
        executing = false;
        emit execedSql();
        queryExecutionState = 4;
        return false;
    }

    OCI_lastenv = nullptr;
    OCI_lastcon = nullptr;
    q.clear();
    q.finish();
    query = nullptr;

    for(int i=0;i<data.headers.size();i++)
        data.setHeaderData(i,Qt::Horizontal,data.headers[i]);

    executionTime = executionEnd.toSecsSinceEpoch() - executionStart.toSecsSinceEpoch();
    executionEnd = QDateTime::currentDateTime();
    if(!nodebug) qDebug() << "Execd";
    executing = false;
    queryExecutionState = 4;

    emit execedSql();
    return true;
}



void DatabaseConnection::stopRunning()
{
    qDebug()<<"attempting to stop query";

    data.stopNow = true;

    for(int i=0;i<asyncExecution_databaseConnections.size();i++)
    {
        if(asyncExecution_databaseConnections[i]!=nullptr && asyncExecution_databaseConnections[i]->executing ||asyncExecution_databaseConnections[i]->dataDownloading)
            asyncExecution_databaseConnections[i]->stopRunning();
    }

    stopNow = true;
    if(subscriptConnesction != nullptr && subscriptConnesction->executing)
    {
        subscriptConnesction->stopRunning();
        stopNow = true;
        return;
    }
    if(dataDownloading)
    {
        stopNow = true;
        dataDownloading = false;
        return;
    }

    else if (customSQlite && sqlite3DBConnectionIsOpen && sqlite3DBConnection!=nullptr)
    {
        sqlite3_interrupt((sqlite3*)sqlite3DBConnection);
    }

    else if(customPSQL && pg_connected && ptr_PGconn != nullptr)
    {
        // just in case
        qDebug() << "sending cancel to postgres";


        qDebug() << "PQrequestCancel returned " << PQrequestCancel((PGconn*) ptr_PGconn);


        //ptr_PGconn = nullptr;
        //pg_connected = false;
        //Create(driver,dbname);
    }

    #ifdef Oracle_OCI_Driver
    else if(customOracle && ((oracle::occi::Environment*)OCI_lastenv)!=nullptr && ((oracle::occi::Connection*)OCI_lastcon)!=nullptr)
    { // signal to stop running query


        qDebug()<<"sending OCIBreak";
        try{
            if( ((oracle::occi::Connection*)OCI_lastcon)!=nullptr  && ((oracle::occi::Environment*)OCI_lastenv)!=nullptr )
            {

                qDebug() << "realy sending OCIBreak(con->getOCIServer(),0);";

                static OCIError *errhp = NULL;
                (void) OCIHandleAlloc( (dvoid *) ((oracle::occi::Environment*)OCI_lastenv)->getOCIEnvironment(), (dvoid **) &errhp, OCI_HTYPE_ERROR, (size_t) 0, (dvoid **) 0);


                int intResult = OCIBreak(((oracle::occi::Connection*)OCI_lastcon)->getOCIServer(), errhp);
                qDebug() << "intResult = OCI_SUCCESS " << intResult  << " = "<< OCI_SUCCESS << OCI_ERROR << OCI_SUCCESS_WITH_INFO << OCI_INVALID_HANDLE;
                //intResult = OCIBreak(((oracle::occi::Connection*)OCI_lastcon)->getOCIServer(), errhp);
                //qDebug() << "serv intResult = OCI_SUCCESS " << intResult  << " = "<< OCI_SUCCESS << OCI_ERROR << OCI_SUCCESS_WITH_INFO << OCI_INVALID_HANDLE;

                try
                {
                    OCIHandleFree(errhp,OCI_HTYPE_ERROR);
                    //con->terminateStatement(stmt);

                    //env->terminateConnection(con);
                }
                catch(oracle::occi::SQLException ex)
                {
                    qDebug() << ex.getErrorCode() << ex.getMessage();
                }

            }
        }
        catch(oracle::occi::SQLException ex)
        {
            qDebug()<<"ERROR at cancel query" << ex.getErrorCode() << ex.what();
        }
        //currentRunningOracleDriver = nullptr;
    }
    #endif
}



// functions for QML
TableData* DatabaseConnection::getData()
{
    return &data;
}

QString DatabaseConnection::replace(QString str,QString what, QString with)
{ // love qml strings
    return str.replace(what,with);
}

QString DatabaseConnection::getDay()
{

    QString str = QVariant(QDate::currentDate().day()).toString();
    if (str.size() < 2)
        return "0" + str;
    else
        return str;
}

QString DatabaseConnection::getMonth()
{
    QString str = QVariant(QDate::currentDate().month()).toString();
    if (str.size() < 2)
        return "0" + str;
    else
        return str;
}

QString DatabaseConnection::getYear()
{
    return QVariant(QDate::currentDate().year()).toString();
}
