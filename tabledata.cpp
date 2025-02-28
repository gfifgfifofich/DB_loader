#include "tabledata.h"

#include <QSqlDatabase>
#include <QSqlError>
#include <QTableView>
#include <QSqlQuery>
#include <QSqlResult>
#include <QSqlRecord>
#include <fstream>
#include <qdir.h>
#include "QXlsx-master/QXlsx/header/xlsxdocument.h"
#include "QXlsx-master/QXlsx/header/xlsxworkbook.h"



TableData::TableData(QObject *parent)
    : QObject{parent}
{
    Init();
}


void TableData::Init()
{
    data.clear();
    headers.clear();
    types.clear();
}

void TableData::ImportFromCSV(QString fileName, QChar delimeter, bool firstRowHeader)
{
    if(fileName.size() < 2)
        return;
    qDebug()<< " importing "<< fileName;

    data.clear();
    headers.clear();
    QFile file(fileName);
    bool first = true;
    if(file.open(QFile::OpenModeFlag::ReadOnly))
    {
        if(firstRowHeader)
        {
            QString str = file.readLine().toStdString().c_str();
            if(str.endsWith('\n'))
                str.resize(str.size()-1);
            if(str.endsWith('\r'))
                str.resize(str.size()-1);
            headers = str.split(delimeter);
            data.resize(headers.size());
        }

        while(!file.atEnd())
        {
            QString str = file.readLine().toStdString().c_str();
            if(str.endsWith('\n'))
                str.resize(str.size()-1);
            if(str.endsWith('\r'))
                str.resize(str.size()-1);
            QStringList strl = str.split(delimeter);
            if(first && !firstRowHeader)
            {
                headers.resize(strl.size());
                data.resize(strl.size());
            }
            first = false;
            if(data.size()>strl.size())
            {
                qDebug()<<"error importing from csv, if(data.size()>strl.size()) is true";
                data.clear();
                headers.clear();
                return;
            }

            for(int i=0;i<data.size();i++)
            {
                data[i].push_back(QVariant(strl[i]));
            }


        }

    }
    else
        qDebug()<< "failed to open "<< fileName;


}
void TableData::ImportFromExcel(QString fileName, int x_start,int x_end,int y_start,int y_end, bool firstRowHeader)
{
    if(fileName.size() < 2)
        return;
    bool serachall = x_start == 0 && y_start == 0 && x_end == 0 && y_end == 0;
    qDebug()<<"TableData::ImportFromExcel Unimplemented";

    data.clear();
    headers.clear();
    QXlsx::Document xlsxR3(fileName);
    qDebug()<< "xlsxR3.load() " << xlsxR3.load();

    qDebug() << "importing excel table from " << fileName;

    QVariant lastcellval = xlsxR3.read(1,1);



    int column = 1;
    while(lastcellval.toString().size()>0)
    {
        headers.push_back(lastcellval.toString());
        column++;
        lastcellval = xlsxR3.read(1,column);
    }
    data.resize(headers.size());
    int row = 1;
    bool allzeros = false;
    while (row <= 1'001'001 && !allzeros)
    {
        bool hasnonzero = false;
        for(int i=0;i<data.size();i++)
        {
            lastcellval = xlsxR3.read(row+1,i+1);
            data[i].push_back(lastcellval);
            if(lastcellval.toString().size()>0)
            {
                hasnonzero = true;
            }
        }
        if(!hasnonzero)
        {
            for(int i=0;i<data.size();i++)
            {
                data[i].pop_back();
                allzeros = true;
            }
        }

        row++;
    }

}
void TableData::ImportFromSQLiteTable(QString fileName, QString tableName)
{
    qDebug()<<"TableData::ImportFromSQLiteTable Unimplemented";
}

bool TableData::ExportToCSV(QString fileName, char delimeter, bool firstRowHeader)
{
    std::fstream stream(fileName.toStdString());
    stream.open(fileName.toStdString(),std::ios_base::out);
    if(stream.is_open())
    {
        for(int i=0;i<headers.size();i++)
        {
            stream << headers[i].toStdString();
            stream << delimeter;
        }
        stream << '\n';
        if(data.size() > 0 && data[0].size() > 0)
            for(int a=0;a<data[0].size();a++)
            {
                for(int i=0;i<data.size();i++)
                {

                    stream << data[i][a].toString().toStdString();
                    stream << delimeter;

                }
                stream << '\n';
            }
    }
    else
    {
        qDebug() <<"failed to open file "<< fileName;
        return false;
    }
    stream.close();
    return true;
}

bool TableData::ExportToExcel(QString fileName, int x_start,int x_end,int y_start,int y_end, bool firstRowHeader)
{
    QXlsx::Document xlsxR3;

    if(data.size()<=0)
        return false;

    qDebug() << "saving";
    int sheetnumber = 1;
    int rowoffset = 0;

    for(int i=0;i<headers.size();i++)
    {
        QVariant var = headers[i];

        xlsxR3.write(1,i+1,var);
    }

    //int datasaved = 0;

    if(data.size() > 0 && data[0].size() <=1'000'000)
        for(int i=0;i<data.size();i++)
        {
            for(int a=0;a<data[i].size();a++)
            {
                int row = a + 2 - rowoffset;
                int column = i + 1;


                xlsxR3.write(row,column,data[i][a]);
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
            for(int i=0;i<data.size();i++)
            {
                for(int a=start ;a<end ;a++)
                {
                    int row = a + 2 - start;
                    int column = i + 1;
                    xlsxR3.write(row,column,data[i][a]);
                }
            }

            if(end >= data[0].size())
                break;
            sheetnumber++;

            start = end;
            end += 1'000'000;
            if(end > data[0].size())
                end = data[0].size();

            QString sheetname = "Sheet";
            sheetname += QVariant(sheetnumber).toString();
            xlsxR3.addSheet(sheetname);
            xlsxR3.selectSheet(sheetname);
            for(int asdi=0;asdi<headers.size();asdi++)
            {
                QVariant var = headers[asdi];
                xlsxR3.write(1,asdi+1,var);
            }
        }
    }
    xlsxR3.addSheet("SQL");
    xlsxR3.selectSheet("SQL");
    xlsxR3.write(1,1,"SQL QUERY");
    xlsxR3.write(2,1,sqlCode );
    xlsxR3.write(3,1,"all SQL Code");
    xlsxR3.write(4,1,allSqlCode );
    xlsxR3.selectSheet("Sheet1");
    if(xlsxR3.saveAs(fileName))
    {
        qDebug()<<"saved.";
        return true;
    }
    else
    {
        qDebug()<<"failed to save, file probably opened.";
        return false;
    }
}

bool TableData::ExportToSQLiteTable(QString tableName)
{
    QSqlDatabase tmpdb = QSqlDatabase::addDatabase("QSQLITE","SQLITE db connection");
    tmpdb.setDatabaseName("SQLiteDB.db");
    tmpdb.open();
    QSqlQuery SQLITE_q(tmpdb);


    QString SQLITE_sql = "Drop table ";
    SQLITE_sql += tableName;
    SQLITE_sql += ";";
    if(SQLITE_q.exec(SQLITE_sql))
        qDebug()<< "Dropped sqlite table";

    SQLITE_sql = "Create table ";
    SQLITE_sql += tableName;

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
        qDebug()<< "Failed to create sqlite table: " <<SQLITE_q.lastError().text();




    SQLITE_sql = "Insert into ";
    SQLITE_sql += tableName;
    SQLITE_sql += " Select ";


    if(data.size()>0 && data[0].size()>0)
    {
        bool first = true;
        for(int a=0;a<data.size();a++)
        {
            if(!first)
                SQLITE_sql += ",";
            first = false;
            SQLITE_sql += " '";
            SQLITE_sql += data[a][0].toString();
            SQLITE_sql += "' ";
            SQLITE_sql += " as ";
            SQLITE_sql += " '";
            SQLITE_sql += headers[a];
            SQLITE_sql += "' ";
        }
    }
    else
    {
        return false;
    }
    qDebug() << SQLITE_sql;
    int lasti=0;
    for(int i=1;i<data[0].size();i++)
    {

        SQLITE_sql += " union all Select ";
        bool first = true;
        for(int a=0;a<data.size();a++)
        {
            if(!first)
                SQLITE_sql += ",";
            first = false;
            int row =i+2;
            int column =a+1;
            bool is_text = false;
            data[a][i].toDouble(&is_text);
            is_text = !is_text;


            SQLITE_sql += " '";
            SQLITE_sql += data[a][i].toString();
            SQLITE_sql += "' ";

        }
        if(i - lasti > 300)
        {
            lasti = i+1;
            if(!SQLITE_q.exec (SQLITE_sql))
                qDebug()<< "Failed to save to sqlite: " <<SQLITE_q.lastError().text();

            SQLITE_sql = "Insert into ";
            SQLITE_sql += tableName;
            SQLITE_sql += " Select ";


            if(data[0].size()>lasti)
            {
                bool first = true;
                for(int a=0;a<data.size();a++)
                {
                    if(!first)
                        SQLITE_sql += ",";
                    first = false;
                    SQLITE_sql += " '";
                    SQLITE_sql += data[a][lasti].toString();
                    SQLITE_sql += "' ";
                    SQLITE_sql += " as ";
                    SQLITE_sql += " '";
                    SQLITE_sql += headers[a];
                    SQLITE_sql += "' ";
                }
            }

            i+=1;
        }
    }
    if(!SQLITE_q.exec (SQLITE_sql))
        qDebug()<< "Failed to save to sqlite: " <<SQLITE_q.lastError().text();
    return true;
}



