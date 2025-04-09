#include "tabledata.h"
#include "sqlSubfunctions.h"

#include <QSqlDatabase>
#include <QSqlError>
#include <QTableView>
#include <QSqlQuery>
#include <QSqlResult>
#include <QSqlRecord>
#include <fstream>
#include <qdir.h>

inline QString documentsDir;
/*

double 6
QString 10
QString 10
double 6
QString 10
QString 10
QString 10
QDateTime 16
double 6
QString 10
*/

TableData::TableData(QObject *parent)
{
    Init();
}

void TableData::Init()
{
    tbldata.clear();
    headers.clear();
    types.clear();
}

void TableData::ImportFromCSV(QString fileName, QChar delimeter, bool firstRowHeader)
{
    if(fileName.size() < 2)
        return;
    qDebug()<< " importing "<< fileName;

    tbldata.clear();
    headers.clear();
    QFile file(fileName);
    bool first = true;
    if(file.open(QFile::OpenModeFlag::ReadOnly))
    {
        if(firstRowHeader)
        {
            QString str = QString().fromUtf8(file.readLine());
            if(str.endsWith('\n'))
                str.resize(str.size()-1);
            if(str.endsWith('\r'))
                str.resize(str.size()-1);
            headers = str.split(delimeter);
            tbldata.resize(headers.size());
        }

        while(!file.atEnd())
        {
            QString str = QString().fromUtf8(file.readLine());
            if(str.endsWith('\n'))
                str.resize(str.size()-1);
            if(str.endsWith('\r'))
                str.resize(str.size()-1);
            QStringList strl = str.split(delimeter);
            if(first && !firstRowHeader)
            {
                headers.resize(strl.size());
                tbldata.resize(strl.size());
            }
            first = false;
            if(tbldata.size()>strl.size())
            {
                qDebug()<<"error importing from csv, if(data.size()>strl.size()) is true";
                tbldata.clear();
                headers.clear();
                return;
            }

            for(int i=0;i<tbldata.size();i++)
            {
                QVariant var = strl[i];
                tbldata[i].push_back(fixQVariantTypeFormat(var));
            }


        }

    }
    else
        qDebug()<< "failed to open "<< fileName;

    for(int i=0;i<headers.size();i++)
        setHeaderData(i,Qt::Horizontal,headers[i]);

}
void TableData::ImportFromExcel(QString fileName, int x_start,int x_end,int y_start,int y_end, bool firstRowHeader)
{
    if(fileName.size() < 2)
        return;
    bool serachall = x_start == 0 && y_start == 0 && x_end == 0 && y_end == 0;
    qDebug()<<"TableData::ImportFromExcel Unimplemented";

    tbldata.clear();
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
    tbldata.resize(headers.size());
    int row = 1;
    bool allzeros = false;
    while (row <= 1'001'001 && !allzeros)
    {
        bool hasnonzero = false;
        for(int i=0;i<tbldata.size();i++)
        {
            lastcellval = xlsxR3.read(row+1,i+1);
            tbldata[i].push_back(lastcellval);
            if(lastcellval.toString().size()>0)
            {
                hasnonzero = true;
            }
        }
        if(!hasnonzero)
        {
            for(int i=0;i<tbldata.size();i++)
            {
                tbldata[i].pop_back();
                allzeros = true;
            }
        }

        row++;
    }
    for(int i=0;i<headers.size();i++)
    {
        setHeaderData(i,Qt::Horizontal,headers[i]);
        qDebug()<<headerData(0,Qt::Horizontal);
    }
}
void TableData::ImportFromSQLiteTable(QString fileName, QString tableName)
{
    qDebug()<<"TableData::ImportFromSQLiteTable Unimplemented";
}

bool TableData::ExportToCSV(QString fileName, char delimeter, bool firstRowHeader)
{
    QFile fl((additionalSaveFileData + fileName));
    fl.open(QFile::OpenModeFlag::WriteOnly);
    if(fl.isOpen())
    {
        for(int i=0;i<headers.size();i++)
        {
            fl.write(headers[i].toUtf8().constData());
            if(i != headers.size()-1)
                fl.write(QString(delimeter).toUtf8().constData());
        }
        fl.write(QString('\n').toUtf8().constData());
        if(tbldata.size() > 0 && tbldata[0].size() > 0)
            for(int a=0;a<tbldata[0].size();a++)
            {
                for(int i=0;i<tbldata.size();i++)
                {
                    QString str = tbldata[i][a].toString();

                    if(str.contains(delimeter))
                    {
                        if(delimeter == ';')
                            str = str.replace(";",",");
                        else
                            str = str.replace(delimeter,";");
                    }
                    if(tbldata[i][a].typeId() == 16)
                    {

                        str.resize(19);
                        str = str.replace("T"," ");
                        fl.write(str.toUtf8().constData());
                    }
                    else if(tbldata[i][a].typeId() == 6)
                    {
                        str = str.replace(".",",");
                        fl.write(str.toUtf8());
                    }
                    else
                        fl.write(str.toUtf8().constData());
                    if(i != tbldata.size()-1) // remove last delimeter in line, .csv should end with \n, not delimeter
                    fl.write(QString(delimeter).toUtf8().constData());

                }
                fl.write(QString('\n').toUtf8().constData());
            }
    }
    else
    {
        qDebug() <<"failed to open file "<< (additionalSaveFileData + fileName);
        return false;
    }
    fl.close();
    return true;
}
bool TableData::ExportToExcel(QString fileName, int x_start,int x_end,int y_start,int y_end, bool firstRowHeader)
{

    if(tbldata.size()<=0)
        return false;
    bool diap = false;
    if(x_start != 0 || x_end != 0 || y_start != 0 || y_end != 0 )
    {// save into diapozon
        diap = true;
        qDebug() << "saving boundries "<< x_start <<  x_end << y_start << y_end;
    }
    // to load only if needed
    QString tmpfilenm = "";
    if(diap) tmpfilenm = fileName;
    QXlsx::Document xlsxR3(tmpfilenm);

    qDebug() << "saving " << fileName;
    int sheetnumber = 1;
    int rowoffset = y_start;
    int column_offset = x_start;
    if(firstRowHeader)
    for(int i=0;i<headers.size();i++)
    {
        QVariant var = headers[i];

        xlsxR3.write(1 + y_start,i+1 + x_start,var);
    }

    //int datasaved = 0;

    if(tbldata.size() > 0 && tbldata[0].size() <=1'000'000 || diap)
    {
        if(diap)
        {
            rowoffset = -y_start;
            column_offset = x_start;
            qDebug()<< diap << x_end - x_start + 1 << y_end - y_start + 1;
        }

        for(int i=0;i<tbldata.size();i++)
        {
            if(diap && i >= x_end - x_start + 1)
                break;
            for(int a=0;a<tbldata[i].size();a++)
            {
                if(diap && a >= y_end - y_start + 1)
                    break;
                int row = a + 2 - rowoffset - (!firstRowHeader);
                int column = i + 1 + column_offset;


                xlsxR3.write(row,column,tbldata[i][a]);

            }
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
            for(int i=0;i<tbldata.size();i++)
            {
                for(int a=start ;a<end ;a++)
                {
                    int row = a + 2 - start - (!firstRowHeader);
                    int column = i + 1;
                    xlsxR3.write(row,column,tbldata[i][a]);
                }
            }

            if(end >= tbldata[0].size())
                break;
            sheetnumber++;

            start = end;
            end += 1'000'000;
            if(end > tbldata[0].size())
                end = tbldata[0].size();

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
    if(xlsxR3.saveAs((additionalSaveFileData + fileName)))
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

        int doublecnt = 0;
        int othercnt = 0;


            SQLITE_sql += " text ";
        if(i+1<headers.size())
            SQLITE_sql += ", ";
    }
    SQLITE_sql += " ); ";
    qDebug()<< SQLITE_sql;

    if(SQLITE_q.exec(SQLITE_sql))
        qDebug()<< "Created sqlite table";
    else
    {
        qDebug()<< "Failed to create sqlite table: " <<SQLITE_q.lastError().text();
        return false;
    }



    SQLITE_sql = "Insert into ";
    SQLITE_sql += tableName;
    SQLITE_sql += " Select ";


    if(tbldata.size()>0 && tbldata[0].size()>0)
    {
        bool first = true;
        for(int a=0;a<tbldata.size();a++)
        {
            if(!first)
                SQLITE_sql += ",";
            first = false;
                SQLITE_sql += " '";
            SQLITE_sql += tbldata[a][0].toString();
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
    for(int i=1;i<tbldata[0].size();i++)
    {

        SQLITE_sql += " union all Select ";
        bool first = true;
        for(int a=0;a<tbldata.size();a++)
        {
            if(!first)
                SQLITE_sql += ",";
            first = false;
            int row =i+2;
            int column =a+1;
            bool is_text = false;
            tbldata[a][i].toDouble(&is_text);
            is_text = !is_text;


                SQLITE_sql += " '";
            SQLITE_sql += tbldata[a][i].toString();
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


            if(tbldata[0].size()>lasti)
            {
                bool first = true;
                for(int a=0;a<tbldata.size();a++)
                {
                    if(!first)
                        SQLITE_sql += ",";
                    first = false;
                        SQLITE_sql += " '";
                    SQLITE_sql += tbldata[a][lasti].toString();
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



// move table(x_s,y_s,x_s,y_s) by dx, dy
// ExcelMove(0,0,10,10, 5,5); // move block 5 right 5 down
bool TableData::ExcelMove(QString table,int x_start,int x_end,int y_start,int y_end, int dx, int dy)
{
    std::vector<std::vector<QVariant>> tmpTbldata;
    QVariant lastcellval;
    QXlsx::Document xlsx(table);

    // form a cube of data
    tmpTbldata.resize(x_end - x_start + 1);
    if(tmpTbldata.size() <=0)
        return false;
    // read cube
    for(int i=0;i<tmpTbldata.size();i++)
    {    for(int a=0;a< y_end - y_start + 1;a++)
        {
            lastcellval = xlsx.read(a + 1 , i + 1);
            tmpTbldata[i].push_back(lastcellval);
            xlsx.write(a + 1 , i + 1,""); // write 0 at position we just read
        }
    }

    for(int i=0;i<tmpTbldata.size();i++)
    {    for(int a=0;a< y_end - y_start + 1;a++)
        {
            xlsx.write(a + 1 + dy , i + 1 + dx, tmpTbldata[i][a]); // write var at position we just read
        }
    }

    if(xlsx.save())
    {
        return true;
    }
    else
    {
        qDebug()<<"Failed to save after move";
        return false;
    }
}


int TableData::getSizeX()
{
    return tbldata.size();
}

int TableData::getSizeY()
{
    if(tbldata.size() > 0)
        return tbldata[0].size();
    else return 0;
}

QVariant TableData::getObject(int x,int y)
{
    //qDebug() << tbldata.size() <<tbldata[0].size() << x << y ;
    //qDebug() << tbldata[x][y].toString();
    if(x>=0 && y>=0 && tbldata.size() > 0 && x < tbldata.size() && y < tbldata[x].size())
    {
        return tbldata[x][y];
    }
    else return QVariant();
}
QVariant TableData::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(section < headers.size())
        return headers[section];
    else return QVariant(section);
}
void TableData::selectXLSXFile(QString filename)
{
    if(xlsxFile != nullptr)
        delete xlsxFile;
    xlsxFile =  new QXlsx::Document(filename);
}
bool TableData::saveXLSXFile(QString filename)
{
    if(xlsxFile != nullptr)
        if(xlsxFile->saveAs(filename))
            return true;
    return false;
}
void TableData::writeToXLSXFile(int x, int y, QVariant value)
{
    if(xlsxFile != nullptr)
    {
        xlsxFile->write(y+1,x+1,value);
    }
}
QVariant TableData::readFromXLSXFile(int x, int y)
{
    if(xlsxFile != nullptr)
    {
        return xlsxFile->read(y+1,x+1);
    }
    else return QVariant();
}

// garbage to bootstrap class imto QML, cuz yea
int TableData::rowCount(const QModelIndex & ) const
{
    if(tbldata.size()<=0)
        return 0;
    else
        return tbldata[0].size();
}

int TableData::columnCount(const QModelIndex &) const
{
    return tbldata.size();
}

QVariant TableData::data(const QModelIndex &index, int role) const
{

    if(tbldata.size() > 0 && index.column() < tbldata.size() && index.row() < tbldata[0].size())
        return tbldata[index.column()][index.row()];
    return QVariant();
}

QHash<int, QByteArray> TableData::roleNames() const
{
    return { {Qt::DisplayRole, "display"} };
}
