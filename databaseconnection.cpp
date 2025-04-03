#include "databaseconnection.h"
#include "datastorage.h"
#include <qapplication.h>
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
    bool QODBC_Excel = false;
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
        else if (driver =="QODBC_Excel")
        {
            db = QSqlDatabase::addDatabase("QODBC",connectionName);
            QODBC_Excel = true;
            ODBC = true;
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

bool DatabaseConnection::Create(QString driver, QString DBName)
{
    if(userDS.Load("userdata.txt"))
    {
        this->driver = driver;
        this->dbname = DBName;
        this->usrname = userDS.data[DBName.toStdString()]["name"].c_str();
        this->password = userDS.data[DBName.toStdString()]["password"].c_str();
        return Create(driver.trimmed(),dbname.trimmed(), usrname.trimmed(), password.trimmed());
    }
    else return false;
}

bool DatabaseConnection::execSql(QString sql)
{
    stopNow = false;// to be shure we wont reject query right after exec
    if(sql.size() <=0)
        sql = sqlCode;
    tableDataMutex.lock();
    tableDataMutex.unlock();
    executionTime = QDateTime::currentSecsSinceEpoch();
    executionStart = QDateTime::currentDateTime();
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
    bool reset = true;
    int loopcount = -1;
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
                            if(justOutOfBrackets && sql[buff_a] != '-'&& sql[buff_a] != '{'&& sql[buff_a] != '}'&& sql[buff_a] != ' '&& sql[buff_a] != '\t'&& sql[buff_a] != '\t'&& sql[buff_a] != '\r')
                                inlineVariables = false;
                            if(inlineVariables)
                            {
                                if(bracketValue > 1 &&  sql[buff_a] != '{'&& sql[buff_a] != '}'&& sql[buff_a] != ' ' && sql[buff_a] != '\t'&& sql[buff_a] != '\t'&& sql[buff_a] != '\r')
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
                        if(subCommandPatterns[i].startsWith("SubexecTo"))
                        {// its a subexec, crop part from next { to }, exec it in subscriptConnection
                            qDebug()<<"Its SubExec";


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

                            QString username = userDS.GetObject(subscriptDBname.toStdString())["name"].c_str();
                            QString password = userDS.GetObject(subscriptDBname.toStdString())["password"].c_str();
                            qDebug()<< subscriptDriver << subscriptDBname << username.trimmed() << password.trimmed();
                            subscriptConnesction->connectionName = connectionName + QVariant(a).toString();
                            subscriptConnesction->Create(subscriptDriver.trimmed(),subscriptDBname.trimmed(),username.trimmed(),password.trimmed());
                            while(scriptCommand.endsWith(' ') ||scriptCommand.endsWith('\t') ||scriptCommand.endsWith('\n') || scriptCommand.endsWith('\r'))
                                scriptCommand.resize(scriptCommand.size()-1);
                            subscriptConnesction->execSql(scriptCommand.trimmed());
                            // silent exporting
                            if(subCommandPatterns[i] == "SubexecToSilentSqliteTable")
                            {
                                qDebug() << "silent exporting to SQLite " << saveName;
                                if(subscriptConnesction != nullptr && subscriptConnesction->data.tbldata.size() > 0)
                                    subscriptConnesction->data.ExportToSQLiteTable(saveName);
                            }
                            if(subCommandPatterns[i] == "SubexecToSilentExcelTable")
                            {
                                qDebug() << "silent exporting to Excel " << saveName;
                                if(subscriptConnesction != nullptr && subscriptConnesction->data.tbldata.size() > 0)
                                    subscriptConnesction->data.ExportToExcel(QString("excel/") + QString(saveName) + QString(".xlsx"),saveStart_X,saveEnd_X,saveStart_Y,saveEnd_Y,true);
                            }
                            if(subCommandPatterns[i] == "SubexecToSilentCSV")
                            {
                                qDebug() << "silent exporting to CSV " << saveName;
                                if(subscriptConnesction != nullptr && subscriptConnesction->data.tbldata.size() > 0)
                                    subscriptConnesction->data.ExportToCSV(QString("CSV/") + QString(saveName) + QString(".csv"),csvDelimeter,true);
                            }
                            // non silent exporting
                            if(subCommandPatterns[i] == "SubexecToSqliteTable")
                            {
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
                            if(subCommandPatterns[i] == "SubexecToExcelTable")
                            {
                                qDebug() << "exporting to Excel " << saveName;
                                QString saveErrorStr = "";
                                if(subscriptConnesction != nullptr && subscriptConnesction->data.tbldata.size() > 0)
                                    if(!subscriptConnesction->data.ExportToExcel(QString("excel/") + QString(saveName) + QString(".xlsx"),saveStart_X,saveEnd_X,saveStart_Y,saveEnd_Y,true))
                                        saveErrorStr = "Failed to save to excel, probably file is opened";

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
                            if(subCommandPatterns[i] == "SubexecToCSV")
                            {
                                qDebug() << "exporting to CSV " << saveName;
                                if(subscriptConnesction != nullptr && subscriptConnesction->data.tbldata.size() > 0)
                                    subscriptConnesction->data.ExportToCSV(QString("CSV/") + QString(saveName) + QString(".csv"),csvDelimeter,true);
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


    while(str.endsWith('\n') || str.endsWith('}') ||str.endsWith('\t') || str.endsWith(' '))
        str.resize(str.size()-1);
    qDebug()<<str;
    qDebug() << "creating query";
    emit queryBeginCreating();

    QSqlQuery q(db);
    query = &q;
    qDebug() << "created query";

    q.setForwardOnly(true);
    data.headers.clear();
    qDebug() << "Prepairing query";
    qDebug() <<QApplication::libraryPaths();
    qDebug() << "executing query";
    emit queryBeginExecuting();
    if(q.exec(str) && q.isSelect())
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

        if(oracle)
        {
            // run the get error query from oracle
            //QString GetErrorSQL = "DECLARE        c   INTEGER := DBMS_SQL.open_cursor (); errorpos integer := -1;     BEGIN        DBMS_SQL.parse (c, '";
            QString GetErrorSQL = "declare    v_count number;    v_bad_sql varchar2(32767) :=         '";
            while(str.endsWith(' ') || str.endsWith('\n')|| str.endsWith('\t')|| str.endsWith(';'))
                str.resize(str.size()-1);
            GetErrorSQL +=  str.replace('\'', "''" );
            //GetErrorSQL += "', DBMS_SQL.native);           DBMS_SQL.close_cursor (c);     EXCEPTION        WHEN OTHERS THEN   errorpos := DBMS_SQL.LAST_ERROR_POSITION();       DBMS_OUTPUT.put_line ('Last Error: ' || DBMS_SQL.LAST_ERROR_POSITION ());  DBMS_SQL.close_cursor (c);  RAISE;  END;";
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

        executionTime = QDateTime::currentSecsSinceEpoch() - executionTime;
        qDebug() << "Execd";
        executing = false;
        emit execedSql();
        return false;
    }

    q.clear();
    q.finish();
    query = nullptr;

    for(int i=0;i<data.headers.size();i++)
        data.setHeaderData(i,Qt::Horizontal,data.headers[i]);
    executionTime = QDateTime::currentSecsSinceEpoch() - executionTime;
    executionEnd = QDateTime::currentDateTime();
    qDebug() << "Execd";
    executing = false;
    emit execedSql();
    return true;
}

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


