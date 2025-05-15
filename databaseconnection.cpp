#include "databaseconnection.h"
#include "datastorage.h"
#include <qapplication.h>
#include <qsqldriver.h>
#include <qsqlerror.h>
#include <qsqlrecord.h>
#include <qthread.h>
#include "Patterns.h"
#include "sqlSubfunctions.h"

// crash on query begin. after import? sqlitesave?
// query success?

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
// really shouldnt've created this class, but yea, here it is. used only when connecting, to delete connection if something wetn wrong
#include "oracledriver.h"


// inline oracle::occi::Environment *((oracle::occi::Environment*)OCI_lastenv) = nullptr;
// inline oracle::occi::Connection *((oracle::occi::Connection*)OCI_lastcon) = nullptr;
// inline oracle::occi::Statement *((oracle::occi::Statement*)OCI_laststmt) = nullptr;

inline OCIServer* lastlastserv;
inline QString lastOracleError = "";


bool DatabaseConnection::Create(QString driver, QString dbname, QString username, QString password)
{
    db.close();
    sqlite = false;
    oracle = false;
    postgre = false;
    ODBC = false;
    customOracle = false;

    bool QODBC_Excel = false;
    QString dbSchemaName = "NoName";

    // decide driver type
    if(driver.trimmed() != "LOCAL_SQLITE_DB")
    {
        if(driver =="QOCI")
        {
            db = QSqlDatabase::addDatabase("QOCI",connectionName);
            db.setConnectOptions("OCI_ATTR_PREFETCH_ROWS=5000");
            oracle = true;
        }
        else if(driver =="QPSQL")
        {
            db = QSqlDatabase::addDatabase("QPSQL",connectionName);
            postgre = true;
        }
        else if (driver =="QODBC_Excel")
        {
            db = QSqlDatabase::addDatabase("QODBC",connectionName);
            QODBC_Excel = true;
            ODBC = true;
        }
        else if (driver =="Oracle")
        {
            oracle = true;
            customOracle = true;
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
            dbname = documentsDir + "/SQLiteDB.db";

        dbSchemaName = dbname.split('.')[0]; // all before first dot
        username = " ";
        password = " ";
        db.setDatabaseName(dbname);
        sqlite = true;
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
            qDebug() << ip << port << dbSchemaName;
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

            }
            else
                connectString = dbname;
        }
        if(!postgre)
            db.setDatabaseName(connectString);
        db.setUserName(username);
        db.setPassword(password);
        qDebug() << connectString;
    }

    // open db
    if(!customOracle)
    {
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
    }
    else
    {

        try
        {
            OracleDriver od;
            QString connection_string ="(DESCRIPTION=(ADDRESS=(PROTOCOL=tcp) (HOST=" + ipAddres.trimmed() + ") (PORT=" + port.trimmed() + "))(CONNECT_DATA=(SERVICE_NAME=" + schemaName.trimmed() + ")))";
            od.Create(username.trimmed().toStdString(),password.trimmed().toStdString(),connection_string.toStdString());

            qDebug() << "db opened";


            this->driver = driver;
            this->dbname = dbname;
            this->usrname =  username;
            this->password = password;

            LastDBName = dbname;
            return true;
        }
        catch (oracle::occi::SQLException ex) {
            qDebug() << "error creating connection" << ex.getErrorCode() << ex.getMessage();
            return false;
        }
    }
    return false;
}

// create db connection using user data
bool DatabaseConnection::Create(QString driver, QString DBName)
{
    if(userDS.Load((documentsDir + "/userdata.txt").toStdString()))
    {
        this->driver = driver;
        this->dbname = DBName;
        this->usrname = userDS.data[DBName.toStdString()]["name"].c_str();
        this->password = userDS.data[DBName.toStdString()]["password"].c_str();
        return Create(driver.trimmed(),dbname.trimmed(), usrname.trimmed(), password.trimmed());
    }
    else return false;
}


//inline OracleDriver* currentRunningOracleDriver = nullptr;

/*Oracle straight up crashes whole app if you try doing that when there are more than some amount of columns. Storing MetaData untill very end will delay crash to when user will want to close app, so works. But app will crash instead of closing if user used Oracle driver(
yep, imma gonna shart into memory, and not deal with crashes when attempting to clear MetaData.
*/
std::vector<std::vector<oracle::occi::MetaData>> mtl;

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
    executionEnd = QDateTime::currentDateTime();
    if(!db.isOpen() && !customOracle)
    {
        bool ok = db.open();
        if(ok)
            qDebug() << "db opened";
        else
            qDebug() << "nope: "<< db.lastError().text().toStdString();
    }

    if(sql.size() <=0)
        sql = sqlCode;

    // Prepeare sql to be run, detect any special keywords
    QString formatedSql;
    QString keywordbuff = "";
    std::vector<int> localKeywordsMatch;
    localKeywordsMatch.resize(subCommandPatterns.size());
    userDS.Load((documentsDir + "/userdata.txt").toStdString());

    // iterate through keywords, detect similarity
    qDebug()<<"Entered sql subcomand processing ";
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
                                qDebug()<< a;
                                break;
                            }
                            if(justOutOfBrackets && sql[buff_a] != '-'&& sql[buff_a] != '{'&& sql[buff_a] != '}'&& sql[buff_a] != ' '&& sql[buff_a] != '\t'&& sql[buff_a] != '\n'&& sql[buff_a] != '\t'&& sql[buff_a] != '\r')
                                inlineVariables = false;
                            if(inlineVariables)
                            {
                                if(bracketValue > 1 &&  sql[buff_a] != '{'&& sql[buff_a] != '}' && sql[buff_a] != '\t'&& sql[buff_a] != '\t'&& sql[buff_a] != '\r')
                                {
                                    lastVar += sql[buff_a];
                                }
                                if(sql[buff_a] == '}')
                                {
                                    lastVar = lastVar.trimmed();
                                    funcVariables  << lastVar;
                                    lastVar = "";
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


                            if(beginSubscriptSQL)
                                scriptCommand.push_back(sql[buff_a]);


                            buff_a++;
                        }
                        a = buff_a;
                        qDebug()<< "subexec variables " << funcVariables;
                        // recursion does its thing, recursive subexec's are possible and infinite^tm
                        if(subCommandPatterns[i].startsWith("Subexec") ||subCommandPatterns[i].startsWith("SilentSubexec") || subCommandPatterns[i].startsWith("ExcelTo")|| subCommandPatterns[i].startsWith("CSVTo"))
                        {// its a subexec, crop part from next { to }, exec it in subscriptConnection
                            qDebug()<<"Its SubExec or ExcelTo or CSVTo";



                            qDebug()<< "";
                            qDebug()<< "Subscript command is:";
                            qDebug()<< scriptCommand;
                            qDebug()<< "";
                            QString subscriptDriver = "NoDriver";
                            QString subscriptDBname = "NoDatabase";
                            QString saveName = "unset_tmp_savename";
                            char csvDelimeter = ';';
                            int saveStart_X = 0;
                            int saveStart_Y = 0;
                            int saveEnd_X = 0;
                            int saveEnd_Y = 0;

                            int varcount = 0;
                            for(auto s : funcVariables)
                            {
                                if(varcount == 0)
                                {
                                    varcount++;
                                    subscriptDriver = s.trimmed();
                                }else if(varcount == 1)
                                {
                                    varcount++;
                                    subscriptDBname = s.trimmed();
                                }
                                else if(varcount == 2)
                                {
                                    varcount++;
                                    saveName = s.trimmed();
                                }
                                else if(varcount == 3)
                                {
                                    varcount++;
                                    saveStart_X = QVariant(s.trimmed()).toInt();
                                    csvDelimeter = s.trimmed()[0].unicode();
                                }
                                else if(varcount == 4)
                                {
                                    varcount++;
                                    saveEnd_X = QVariant(s.trimmed()).toInt();
                                }
                                else if(varcount == 5)
                                {
                                    varcount++;
                                    saveStart_Y = QVariant(s.trimmed()).toInt();;
                                }
                                else if(varcount == 6)
                                {
                                    varcount++;
                                    saveEnd_Y = QVariant(s.trimmed()).toInt();
                                }
                                else
                                    break;
                            }

                            qDebug()<< "subscriptDriver: " << subscriptDriver;
                            qDebug()<< "subscriptDBname: " << subscriptDBname;
                            subscriptConnesction = new DatabaseConnection();


                            // subCommandPatterns.push_back("ExcelToMagic");
                            // subCommandPatterns.push_back("ExcelToArray");
                            // if its subexec, subexec, else load from excel file
                            if(subCommandPatterns[i].startsWith("Subexec") ||subCommandPatterns[i].startsWith("SilentSubexec"))
                            {
                                QString username = userDS.GetObject(subscriptDBname.toStdString())["name"].c_str();
                                QString password = userDS.GetObject(subscriptDBname.toStdString())["password"].c_str();
                                qDebug()<< subscriptDriver << subscriptDBname << username.trimmed() << password.trimmed();
                                subscriptConnesction->connectionName = connectionName + QVariant(a).toString();
                                subscriptConnesction->Create(subscriptDriver.trimmed(),subscriptDBname.trimmed(),username.trimmed(),password.trimmed());
                                while(scriptCommand.endsWith(' ') ||scriptCommand.endsWith('\t') ||scriptCommand.endsWith('\n') || scriptCommand.endsWith('\r'))
                                    scriptCommand.resize(scriptCommand.size()-1);

                                subscriptConnesction->execSql(scriptCommand.trimmed());
                            }
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
                                    qDebug() <<"subscriptDriver" <<subscriptDriver;
                                }

                                subscriptConnesction->data.ImportFromExcel(subscriptDriver,0,0,0,0,true);
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
                                    qDebug() <<"subscriptDriver" <<subscriptDriver;
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
                            if(subCommandPatterns[i] == "SilentSubexecToSqliteTable")
                            {
                                savefilecount++;
                                qDebug() << "silent exporting to SQLite " << saveName;

                                if(subscriptConnesction->data.headers.size() > 0 && !subscriptConnesction->data.headers[0].startsWith("Error"))
                                    if(subscriptConnesction != nullptr && subscriptConnesction->data.tbldata.size() > 0)
                                        subscriptConnesction->data.ExportToSQLiteTable(saveName);
                            }
                            else if(subCommandPatterns[i] == "SilentSubexecToExcelTable")
                            {
                                savefilecount++;
                                qDebug() << "silent exporting to Excel " << saveName;

                                if(subscriptConnesction->data.headers.size() > 0 && !subscriptConnesction->data.headers[0].startsWith("Error"))
                                    if(subscriptConnesction != nullptr && subscriptConnesction->data.tbldata.size() > 0)
                                        subscriptConnesction->data.ExportToExcel(QString(documentsDir + "/" + "excel/") + QString(saveName) + QString(".xlsx"),saveStart_X,saveEnd_X,saveStart_Y,saveEnd_Y,true);
                            }
                            else if(subCommandPatterns[i] == "SilentSubexecToCSV")
                            {
                                savefilecount++;
                                qDebug() << "silent exporting to CSV " << saveName;
                                if(subscriptConnesction->data.headers.size() > 0 && !subscriptConnesction->data.headers[0].startsWith("Error"))
                                    if(subscriptConnesction != nullptr && subscriptConnesction->data.tbldata.size() > 0)
                                        subscriptConnesction->data.ExportToCSV(QString(documentsDir + "/" +"CSV/") + QString(saveName) + QString(".csv"),csvDelimeter,true);
                            }
                            else if(subCommandPatterns[i] == "SilentSubexecAppendCSV")
                            {
                                savefilecount++;
                                qDebug() << "silent exporting(Append) to CSV " << saveName;
                                if(subscriptConnesction->data.headers.size() > 0 && !subscriptConnesction->data.headers[0].startsWith("Error"))
                                    if(subscriptConnesction != nullptr && subscriptConnesction->data.tbldata.size() > 0)
                                        subscriptConnesction->data.AppendToCSV(QString(documentsDir + "/" +"CSV/") + QString(saveName) + QString(".csv"),csvDelimeter);
                            }

                            // non silent exporting
                            else if(subCommandPatterns[i] == "SubexecToSqliteTable")
                            {
                                savefilecount++;
                                qDebug() << "exporting to SQLite " << saveName;
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
                                        formatedSql += subscriptConnesction->data.tbldata[0][0].toString();
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
                                }
                            }
                            else if(subCommandPatterns[i] == "SubexecToExcelTable")
                            {
                                savefilecount++;
                                qDebug() << "exporting to Excel " << saveName;
                                QString saveErrorStr = "";

                                if(subscriptConnesction->data.headers.size() > 0 && !subscriptConnesction->data.headers[0].startsWith("Error"))
                                    if(subscriptConnesction != nullptr && subscriptConnesction->data.tbldata.size() > 0)
                                        if(!subscriptConnesction->data.ExportToExcel(QString(documentsDir + "/" +"excel/") + QString(saveName) + QString(".xlsx"),saveStart_X,saveEnd_X,saveStart_Y,saveEnd_Y,true))
                                            saveErrorStr = "Failed to save to excel, probably file is opened";

                                else if (saveErrorStr.size() > 0)
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
                                    formatedSql += subscriptConnesction->data.tbldata[0][0].toString();
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
                            }
                            else if(subCommandPatterns[i] == "SubexecToCSV")
                            {
                                savefilecount++;
                                qDebug() << "exporting to CSV " << saveName;
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
                                    formatedSql += subscriptConnesction->data.tbldata[0][0].toString();
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

                            }
                            else if(subCommandPatterns[i] == "SubexecAppendCSV")
                            {
                                savefilecount++;
                                qDebug() << "exporting (Append) to CSV " << saveName;

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
                                    formatedSql += subscriptConnesction->data.tbldata[0][0].toString();
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

                            }
                            // script extensions/dblinks
                            else if(subCommandPatterns[i] == "SubexecToUnionAllTable")
                            {// exec into union all table
                                // Oracle: select * from dual union all select * from dual
                                // PostgreSQL/SQLite select * union all select *...
                                savefilecount++;
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
                                        if(oracle)
                                            formatedSql += "from dual";
                                        for(int j=1;j<subscriptConnesction->data.tbldata[0].size();j++)
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
                                qDebug() <<"Reached SubexecToMagic implementation";

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
                                                formatedSql += subscriptConnesction->data.tbldata[t][j].toString();
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
                                qDebug() <<"Reached SubexecToArray implementation";
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
                                                formatedSql += subscriptConnesction->data.tbldata[t][j].toString();
                                                formatedSql += "' ";
                                                if(((TableCutoffEnd>=0 && TableCutoffStart>=0  && j+1 <= TableCutoffEnd && j + 1 <subscriptConnesction->data.tbldata[0].size()) || (TableCutoffEnd<0 && TableCutoffStart<0 && j + 1 <subscriptConnesction->data.tbldata[0].size())) || (t + 1 <subscriptConnesction->data.tbldata.size() && neededMagicColumn == -1)) // there will be next
                                                    formatedSql += ",";
                                            }
                                        }
                                    }
                                }




                            }
                            else if(subCommandPatterns[i] == "ExcelToSqliteTable"|| subCommandPatterns[i] == "CSVToSqliteTable")
                            {

                                if(subscriptConnesction->data.headers.size() > 0)
                                    if(subscriptConnesction != nullptr && subscriptConnesction->data.tbldata.size() > 0)
                                        subscriptConnesction->data.ExportToSQLiteTable(subscriptDBname);
                            }
                            subscriptConnesction->db.close();
                            delete subscriptConnesction;
                            subscriptConnesction = nullptr;

                            if(stopNow)
                            {
                                stopNow=false;

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

                            qDebug() << "sending :";
                            qDebug() << "host = " << host;
                            qDebug() << "Sender = " << Sender;
                            qDebug() << "SenderName = " << SenderName;
                            qDebug() << "to = " << to;
                            qDebug() << "cc = " << cc;
                            qDebug() << "Subject = " << Subject;
                            qDebug() << "messageText = " << messageText;
                            qDebug() << "attachments = " << attachments;

                            emit sendMail(host,Sender,SenderName,to,cc,Subject,messageText,attachments);


                        }

                        // no recursion = manualy unroll though reset
                        if(subCommandPatterns[i].trimmed() == "ForLoop" || subCommandPatterns[i].trimmed() == "QueryForLoop")
                        {
                            reset = true;


                            qDebug()<< "ForLoop";
                            qDebug()<< "variables " << funcVariables;
                            qDebug()<< "Subscript command is:";
                            qDebug()<< scriptCommand;
                            qDebug()<< "";
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
                                if(fillStage  == 0)
                                {
                                    fillStage++;
                                    replValue = s.trimmed();
                                }else if(fillStage  == 1)
                                {
                                    fillStage++;
                                    replStart = QVariant(s.trimmed()).toInt(&toIntStart);
                                    valueRuquieredSize = s.trimmed().size();
                                    if(!toIntStart)
                                    {
                                        iterateValues = s.trimmed().split(',');
                                        if(iterateValues.size()>0)
                                            isIteration = true;
                                    }

                                }else if(fillStage  == 2)
                                {
                                    fillStage++;
                                    replEnd = QVariant(s.trimmed()).toInt();
                                }else if(fillStage  == 3)
                                {
                                    fillStage++;
                                    replStep = QVariant(s.trimmed()).toInt();
                                }
                                else
                                    break;
                            }
                            qDebug() << replValue << replStart << replEnd << replStep << "valueRuquieredSize " << valueRuquieredSize;

                            if(!isIteration)
                            {for(int iter = replStart; iter <= replEnd; iter += replStep)
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
                                    formatedSql += fmiterSql;
                                    qDebug()<<iter << fmiterSql;
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
                                    qDebug()<<val << fmiterSql;
                                }
                            }

                            formatedSql += " ";
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
                                    monthnum +=offset;
                            }
                            str = QVariant(monthnum).toString();
                            if(str.size() == 1)
                                str = "0" + str;
                            qDebug() << str;
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
                                    monthnum +=offset;
                            }
                            str = QVariant(monthnum).toString();
                            qDebug() << str;
                            formatedSql += str;
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
        qDebug()<< "";
        qDebug()<< "";
        qDebug()<< "";
        qDebug()<< "loopcount " << loopcount << " sql: "<< sql;
        qDebug()<< "";
        qDebug()<< "";
        qDebug()<< "";
        if(loopcount > 5) break;
    }
    qDebug()<<"Exiting sql subcomand processing........";
    qDebug()<<"Resulting formatedSql is";
    QString str = formatedSql.trimmed();
    executing = true;
    // run result
    while(str.endsWith('\n') || str.endsWith('}') ||str.endsWith('\t') || str.endsWith(' '))
        str.resize(str.size()-1);
    qDebug()<<str;
    qDebug() << "creating query";
    emit queryBeginCreating();

    queryExecutionState = 1;
    if(customOracle)
    {

        emit queryBeginExecuting();
        try{

            OCI_lastenv = oracle::occi::Environment::createEnvironment ( "CL8MSWIN1251", "CL8MSWIN1251",oracle::occi::Environment::Mode::DEFAULT);//oracle::occi::Environment::DEFAULT);

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

                mtl.push_back(rset->getColumnListMetaData());
                std::vector<int> types;
                int type = 0;
                qDebug() << mtl.back().size();
                types.resize(mtl.back().size());
                data.headers.clear();
                data.headers.resize(mtl.back().size());
                data.tbldata.clear();
                data.tbldata.resize(mtl.back().size());
                for(int i=0;i < mtl.back().size(); i++)
                {
                    types[i] = detectType(mtl.back()[i].getInt(oracle::occi::MetaData::ATTR_DATA_TYPE));
                    data.headers[i] = QString().fromLocal8Bit(mtl.back()[i].getString(oracle::occi::MetaData::ATTR_NAME).c_str());
                }


                int ccnt = 0;



                while (rset->next () && mtl.back().size() > 0)
                {
                    ccnt ++;
                     for(int i=0;i < mtl.back().size(); i ++)
                     {
                         if(types[i]!=16)
                         {
                             data.tbldata[i].push_back(fixQVariantTypeFormat(QString().fromLocal8Bit(QByteArray::fromStdString(rset->getString(i+1)))));
                         }
                         else
                            if(!rset->isNull(i + 1))
                            {
                               data.tbldata[i].push_back(fixQVariantTypeFormat(fromOCIDateTime(rset->getDate(i+1))));
                            }
                            else
                            {
                               data.tbldata[i].push_back(QVariant());
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

                dataDownloading = false;
                if(rset!=nullptr)
                    rset->cancel();
                if(((oracle::occi::Statement*)OCI_laststmt)!=nullptr)
                    ((oracle::occi::Statement*)OCI_laststmt)->closeResultSet (rset);
            }catch(oracle::occi::SQLException ex)
            {
                lastLaunchIsError = true;
                qDebug()<<"oracle::occi::SQLException at runSelect" << " Error number: "<<  ex.getErrorCode()  << "    "<<ex.getMessage() ;

                int errorpos = 0; //
                status = false;
                lastOracleError = ex.getMessage().c_str();
                if(((oracle::occi::Statement*)OCI_laststmt)!= nullptr && ((oracle::occi::Environment*)OCI_lastenv) != nullptr)
                {
                    try
                    {
                        static OCIError *errhp = NULL;
                        qDebug() << "generating error";
                        (void) OCIHandleAlloc( (dvoid *) ((oracle::occi::Environment*)OCI_lastenv)->getOCIEnvironment(), (dvoid **) &errhp, OCI_HTYPE_ERROR, (size_t) 0, (dvoid **) 0);



                        qDebug() << "retrieving error";
                        OCIAttrGet((dvoid *)((oracle::occi::Statement*)OCI_laststmt)->getOCIStatement(), (ub4)OCI_HTYPE_STMT,
                                   (dvoid *)&errorpos, (ub4 *)NULL,
                                   (ub4)OCI_ATTR_PARSE_ERROR_OFFSET, errhp);

                        qDebug() << "lastErrorPos = errorpos";
                        lastErrorPos = errorpos;
                        qDebug() <<"errpos" << errorpos;

                        OCIHandleFree(errhp,OCI_HTYPE_ERROR);
                    }
                    catch(oracle::occi::SQLException ex)
                    {
                        qDebug() << ex.getErrorCode() << ex.getMessage();
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

                        qDebug()<<"oracle::occi::SQLException at runSelect" << " Error number: "<<  ex.getErrorCode()  << "    "<<ex.getMessage() ;
                    }

                    data.headers.clear();
                    data.tbldata.clear();
                    data.tbldata.emplace_back();
                    data.headers.push_back("Error");
                    data.tbldata.back().emplace_back("user canceled query");
                    qDebug() << "stopped";
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

            qDebug() << "closing statement";
            if(((oracle::occi::Statement*)OCI_laststmt)!= nullptr)
                ((oracle::occi::Connection*)OCI_lastcon)->terminateStatement (((oracle::occi::Statement*)OCI_laststmt));
            OCI_laststmt = nullptr;

            qDebug() << "closing connection";
            if(((oracle::occi::Environment*)OCI_lastenv)!= nullptr && ((oracle::occi::Connection*)OCI_lastcon) != nullptr)
                ((oracle::occi::Environment*)OCI_lastenv)->terminateConnection (((oracle::occi::Connection*)OCI_lastcon));

            qDebug() << "closing env";
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

                            qDebug()<<"line num is "<< linenum ;
                        }
                    }
                    data.tbldata.back().push_back(errStr);
                }
                data.tbldata.back().push_back(lastOracleError);
                data.tbldata.back().push_back(errStr);

                data.tbldata.emplace_back();
                data.tbldata.back().push_back(QString("Error position ") + QVariant(lastErrorPos + _code_start_pos).toString());


                qDebug() << "Execd";
                OCI_lastenv = nullptr;
                OCI_lastcon = nullptr;
                executing = false;
                executionEnd = QDateTime::currentDateTime();
                executionTime = executionEnd.toSecsSinceEpoch() - executionStart.toSecsSinceEpoch();
                emit execedSql();
                queryExecutionState = 4;
                return false;
            }
            OCI_lastenv = nullptr;
            OCI_lastcon = nullptr;

            executing = false;
            executionEnd = QDateTime::currentDateTime();
            executionTime = executionEnd.toSecsSinceEpoch() - executionStart.toSecsSinceEpoch();
            emit execedSql();
            queryExecutionState = 4;
            return true;
        }
        catch (oracle::occi::SQLException ex){
            qDebug() << "oracledb error";
            lastLaunchIsError = true;
            lastErrorPos = -1;
            qDebug() << "error creating connection" << ex.getErrorCode() << ex.getMessage();
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

                        qDebug()<<"line num is "<< linenum ;
                    }
                }
                data.tbldata.back().push_back(errStr);
            }
            data.tbldata.back().push_back(ex.getMessage().c_str());
            data.tbldata.back().push_back(errStr);

            executionTime = executionEnd.toSecsSinceEpoch() - executionStart.toSecsSinceEpoch();
            //currentRunningOracleDriver = nullptr;

            qDebug() << "Execd";
            OCI_lastenv = nullptr;
            OCI_lastcon = nullptr;
            executing = false;
            executionEnd = QDateTime::currentDateTime();
            executionTime = executionEnd.toSecsSinceEpoch() - executionStart.toSecsSinceEpoch();
            emit execedSql();
            queryExecutionState = 4;
            return false;
        }

        qDebug() << "oracledb execed";

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
    OCI_lastenv = nullptr;
    OCI_lastcon = nullptr;
    //currentRunningOracleDriver = nullptr;
    QSqlQuery q(db);
    query = &q;
    qDebug() << "created query";

    q.setForwardOnly(true);
    data.headers.clear();
    qDebug() << "executing query";
    emit queryBeginExecuting();
    queryExecutionState = 2;
    if(q.exec(str) && q.isSelect())
    {
        qDebug() << "query sucess.  isSelect = " << q.isSelect();
        emit querySuccess();
        queryExecutionState = 3;
        for(int a=0,total = q.record().count(); a<total;a++)
            data.headers << q.record().fieldName(a);
        int i=0;
        data.tbldata.clear();
        data.tbldata.resize(data.headers.size());
        while(q.next())
        {
            for(int a=0,total = q.record().count(); a<total;a++)
            {

                QVariant var = fixQVariantTypeFormat(q.value(a));


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
                    data.tbldata.back().push_back(q.value(0));
                    data.tbldata.back().push_back(q.value(1));
                    data.tbldata.back().push_back(q.value(2));
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

                        qDebug()<<"line num is "<< linenum ;
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
            qDebug() << "Error in sql query:"<< q.lastError().text().toStdString().c_str();
            qDebug() << "Error from database:"<< tr(q.lastError().databaseText().toStdString().c_str());
            qDebug() << "Error from driver:"<< q.lastError().driverText();
            qDebug() << sql;
            data.tbldata.back().push_back(q.lastError().text().toStdString().c_str());
            data.tbldata.back().push_back(q.lastError().databaseText());
            data.tbldata.back().push_back(q.lastError().driverText());
        }

        q.clear();
        q.finish();
        query = nullptr;

        executionEnd = QDateTime::currentDateTime();
        executionTime = executionEnd.toSecsSinceEpoch() - executionStart.toSecsSinceEpoch();
        qDebug() << "Execd";
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
    qDebug() << "Execd";
    executing = false;
    queryExecutionState = 4;
    emit execedSql();
    return true;
}



void DatabaseConnection::stopRunning()
{
    qDebug()<<"attempting to stop query";
    stopNow = true;
    if(subscriptConnesction != nullptr && subscriptConnesction->executing)
    {
        subscriptConnesction->stopRunning();
        stopNow = true;
        //return;
    }

    if(!dataDownloading && customOracle && ((oracle::occi::Environment*)OCI_lastenv)!=nullptr && ((oracle::occi::Connection*)OCI_lastcon)!=nullptr)
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
                intResult = OCIBreak(((oracle::occi::Connection*)OCI_lastcon)->getOCIServer(), errhp);
                qDebug() << "serv intResult = OCI_SUCCESS " << intResult  << " = "<< OCI_SUCCESS << OCI_ERROR << OCI_SUCCESS_WITH_INFO << OCI_INVALID_HANDLE;

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








