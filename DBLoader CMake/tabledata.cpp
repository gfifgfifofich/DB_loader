#include "tabledata.h"
#include "datastorage.h"
#include "loaderwidnow.h"
#include "sqlSubfunctions.h"

#include <QSqlDatabase>
#include <QSqlError>
#include <QTableView>
#include <QSqlQuery>
#include <QSqlResult>
#include <QSqlRecord>
#include <fstream>
#include <qdir.h>
#include "databaseconnection.h"
#include "xlsxstyles_p.h"
#include <QInputDialog>
#include <QMessageBox>
#include <map>
inline QString documentsDir;
inline DataStorage userDS;

/* there is absolutely a better way of checking for type info, but yes. +debt
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
    maxVarTypes.clear();
    typecount.clear();
}



void TableData::FixTypeInfo()
{
    typecount.clear();
    typecount.resize( tbldata.size());
    for(int i=0;i<tbldata.size();i++)
    {
        for(int a = 0; a < tbldata[i].size();a++)
        {
            tbldata[i][a] = fixQStringType(tbldata[i][a]);

            while(typecount[i].size() <= fixQStringType_lasttype)
                typecount[i].emplace_back();

            if(HasLetters(tbldata[i][a]))
                fixQStringType_lasttype=10;

            typecount[i][fixQStringType_lasttype]++;
        }
    }


    maxVarTypes.clear();
    maxVarTypes.resize( typecount.size());


    for(int i=0;i < typecount.size();i++)
    {
        int maxvt = 0;
        for(int a  = 0; a < typecount[i].size();a++)
        {
            if(a == 0 || maxvt <= typecount[i][a])
            {
                maxVarTypes[i] = a;
                maxvt = typecount[i][a];
            }
        }
    }
    for(int i=0;i < typecount.size();i++)
    {
        if(typecount[i].size()>10)
        {
            if(typecount[i][10] > 0)
                maxVarTypes[i] = 10;

        }
    }
    for(int i=0;i<headers.size();i++)
    {
        setHeaderData(i,Qt::Horizontal,headers[i]);
        //qDebug()<<headerData(0,Qt::Horizontal);
    }
}

// import
bool TableData::ImportFromCSV(QString fileName, QChar delimeter, bool firstRowHeader)
{
    stopNow = false;
    if(fileName.size() < 2)
        return false;
    qDebug()<< " importing "<< fileName;

    if(tbldata.size()>0 && tbldata[0].size()>0 && !silentExcelImport)
    {
        // ask user if he wants to merge table data

        if(QMessageBox::question(loadWind, "Merge?", "Do you want to merge with existing data?",QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
        {
            TableData td;
            td.ImportFromCSV(fileName,delimeter,firstRowHeader);

            bool ok = false;
            QInputDialog id(loadWind);
            id.setLabelText("Current column");
            id.setComboBoxItems(headers);
            ok = id.exec();
            if(!ok)
                return false;
            QString header1 = id.textValue();

            ok = false;
            QInputDialog idd(loadWind);
            idd.setLabelText("Column in file");
            idd.setComboBoxItems(td.headers);
            ok = idd.exec();
            if(!ok)
                return false;
            QString header2 = idd.textValue();

            int id1 = -1;
            int id2 = -1;
            for(int i =0;i<headers.size();i++)
                if(headers[i]==header1)
                {
                    id1 = i;
                    break;
                }
            for(int i =0;i<td.headers.size();i++)
                if(td.headers[i]==header2)
                {
                    id2 = i;
                    break;
                }
            if(id1<0 || id2 <0)
                return false;

            int sizebuf = tbldata.size();
            tbldata.resize(tbldata.size()+ td.tbldata.size());
            for(int i = sizebuf; i < tbldata.size();i++)
                tbldata[i].resize(tbldata[0].size());

            headers.resize(headers.size()+ td.headers.size());
            for(int i = sizebuf; i < headers.size();i++)
                headers[i] = td.headers[i - sizebuf];

            std::map<QString,int> idmap;
            for(int i =0;i < tbldata[id1].size();i++)
            {
                idmap[tbldata[id1][i]] = i;
                //qDebug() <<"idmap" << td.tbldata[id1][i] << idmap[tbldata[id1][i]];
            }
            std::map<QString,int> idmap2;
            for(int i =0;i < td.tbldata[id2].size();i++)
            {
                idmap2[td.tbldata[id2][i]] = i;
                //qDebug() <<"idmap2" << td.tbldata[id2][i] << idmap2[td.tbldata[id2][i]];
            }

            qDebug() << idmap2.size() << idmap.size();
            for(auto x : idmap)
            {
                if(idmap2.count(x.first) )
                {
                    int i = x.second;
                    int a = idmap2[x.first];
                    //qDebug() << x.first << " joined " << i << a;
                    for(int j = 0;j < td.tbldata.size();j++)
                        tbldata[sizebuf + j][i] = td.tbldata[j][a];
                }
            }

            // for(int i =0;i < tbldata[id1].size();i++)
            // {
            //     for(int a =0;a < td.tbldata[id2].size();a++)
            //     {
            //         if(td.tbldata[id2][a] == tbldata[id1][i])
            //         {
            //             for(int j = 0;j < td.tbldata.size();j++)
            //                 tbldata[sizebuf + j][i] = td.tbldata[j][a];
            //             break;
            //         }
            //     }
            // }

            FixTypeInfo();
            return true;
        }
    }

    tbldata.clear();
    headers.clear();
    maxVarTypes.clear();
    typecount.clear();
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
            typecount.resize(headers.size());
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
            while(tbldata.size()>strl.size())
            {
                if(file.atEnd())
                {
                    qDebug()<<"error importing from csv, if(data.size()>strl.size()) is true";
                    tbldata.clear();
                    headers.clear();
                    return false;
                }
                else
                    strl += QString().fromUtf8(file.readLine()).split(delimeter);
            }

            for(int i=0;i<tbldata.size();i++)
            {
                tbldata[i].push_back(fixQStringType(strl[i]));

                while(typecount[i].size() <= fixQStringType_lasttype)
                    typecount[i].emplace_back();

                typecount[i][fixQStringType_lasttype]++;
            }

            maxVarTypes.clear();
            maxVarTypes.resize( typecount.size());

            for(int i=0;i < typecount.size();i++)
            {
                int maxvt = 0;
                for(int a  = 0; a < typecount[i].size();a++)
                {
                    if(a == 0 || maxvt <= typecount[i][a])
                    {
                        maxVarTypes[i] = a;
                        maxvt = typecount[i][a];
                    }
                }
            }

        }

    }
    else
        qDebug()<< "failed to open "<< fileName;

    for(int i=0;i<headers.size();i++)
        setHeaderData(i,Qt::Horizontal,headers[i]);
    FixTypeInfo();
    return true;
}
bool TableData::ImportFromExcel(QString fileName, int x_start,int x_end,int y_start,int y_end, bool firstRowHeader,QString sheetName)
{
    stopNow = false;
    if(fileName.size() < 2)
        return false;
    bool serachall = x_start == 0 && y_start == 0 && x_end == 0 && y_end == 0;

    QXlsx::Document xlsxR3(fileName);
    qDebug() << "importing excel table from " << fileName;
    if(!xlsxR3.load())
    {
        qDebug()<< "xlsxR3.load() failed";
        return false;
    }
    else
        qDebug()<< "xlsxR3.load() success";


    int sheetid = 0;
    QStringList sheets = xlsxR3.sheetNames();
    LastExcelImportSheets = xlsxR3.sheetNames();
    bool userControlled = false;

    if(!silentExcelImport && sheetName == "")
        userControlled = true;
    if(sheets.size()>1 && !silentExcelImport && sheetName == "")
    {
        bool ok = true;
        QInputDialog id(loadWind);
        id.setLabelText("Select sheet to import");
        id.setComboBoxItems(sheets);
        id.exec();

        QString text = id.textValue();
        xlsxR3.selectSheet(text);
        sheetName = text;
        qDebug() << text;
        if(!ok)
        {
            qDebug() << "xlsx import failed, user cancel";
            return false;
        }
    }
    else if (sheets.size()>1 && sheetName != "")
    {
        xlsxR3.selectSheet(sheetName);
    }
    else if(sheets.size()<=0)
    {
        qDebug() << "xlsx import failed, no sheets in excel table";
        return false;
    }



    if(userControlled && tbldata.size()>0 && tbldata[0].size()>0&& !silentExcelImport)
    {
        // ask user if he wants to merge table data

        if(QMessageBox::question(loadWind, "Merge?", "Do you want to merge with existing data?",QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
        {
            TableData td;
            td.ImportFromExcel(fileName,x_start,x_end,y_start,y_end,firstRowHeader,sheetName);

            bool ok = false;
            QInputDialog id(loadWind);
            id.setLabelText("Current column");
            id.setComboBoxItems(headers);
            ok = id.exec();
            if(!ok)
                return false;
            QString header1 = id.textValue();

            ok = false;
            QInputDialog idd(loadWind);
            idd.setLabelText("Column in file");
            idd.setComboBoxItems(td.headers);
            ok = idd.exec();
            if(!ok)
                return false;
            QString header2 = idd.textValue();

            int id1 = -1;
            int id2 = -1;
            for(int i =0;i<headers.size();i++)
                if(headers[i]==header1)
                {
                    id1 = i;
                    break;
                }
            for(int i =0;i<td.headers.size();i++)
                if(td.headers[i]==header2)
                {
                    id2 = i;
                    break;
                }
            if(id1<0 || id2 <0)
                return false;

            int sizebuf = tbldata.size();
            tbldata.resize(tbldata.size()+ td.tbldata.size());
            for(int i = sizebuf; i < tbldata.size();i++)
                tbldata[i].resize(tbldata[0].size());

            headers.resize(headers.size()+ td.headers.size());
            for(int i = sizebuf; i < headers.size();i++)
                headers[i] = td.headers[i - sizebuf];

            std::map<QString,int> idmap;
            for(int i =0;i < tbldata[id1].size();i++)
            {
                idmap[tbldata[id1][i]] = i;
                //qDebug() <<"idmap" << td.tbldata[id1][i] << idmap[tbldata[id1][i]];
            }
            std::map<QString,int> idmap2;
            for(int i =0;i < td.tbldata[id2].size();i++)
            {
                idmap2[td.tbldata[id2][i]] = i;
                //qDebug() <<"idmap2" << td.tbldata[id2][i] << idmap2[td.tbldata[id2][i]];
            }

            qDebug() << idmap2.size() << idmap.size();
            for(auto x : idmap)
            {
                if(idmap2.count(x.first) )
                {
                    int i = x.second;
                    int a = idmap2[x.first];
                    //qDebug() << x.first << " joined " << i << a;
                    for(int j = 0;j < td.tbldata.size();j++)
                        tbldata[sizebuf + j][i] = td.tbldata[j][a];
                }
            }




            FixTypeInfo();
            return true;
        }
    }

    tbldata.clear();
    headers.clear();
    typecount.clear();
    maxVarTypes.clear();

    QVariant lastcellval = xlsxR3.read(1,1);
    int column = 1;
    while(lastcellval.toString().size()>0)
    {
        headers.push_back(lastcellval.toString());


        column++;
        lastcellval = xlsxR3.read(1,column);
    }
    tbldata.resize(headers.size());
    typecount.resize(headers.size());
    int row = 1;
    bool allzeros = false;
    while (row <= 1'001'001 && !allzeros)
    {
        bool hasnonzero = false;
        for(int i=0;i<tbldata.size();i++)
        {
            lastcellval = xlsxR3.read(row+1,i+1);
            tbldata[i].push_back(fixQStringType(lastcellval.toString()));

            // force text due to dirty excel tables
            while(typecount[i].size() <= 10)
                typecount[i].emplace_back();

            typecount[i][10]++;

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

    maxVarTypes.clear();
    maxVarTypes.resize( typecount.size());

    for(int i=0;i < typecount.size();i++)
    {
        int maxvt = 0;
        for(int a  = 0; a < typecount[i].size();a++)
        {
            if(a == 0 || maxvt <= typecount[i][a])
            {
                maxVarTypes[i] = a;
                maxvt = typecount[i][a];
            }
        }
    }

    for(int i=0;i<headers.size();i++)
    {
        setHeaderData(i,Qt::Horizontal,headers[i]);
        //qDebug()<<headerData(0,Qt::Horizontal);
    }

    FixTypeInfo();
    return true;
}
void TableData::ImportFromSQLiteTable(QString fileName, QString tableName)
{
    stopNow = false;
    // mostly  pointless in this sort of application
    qDebug()<<"TableData::ImportFromSQLiteTable Unimplemented";
}

// export
bool TableData::ExportToCSV(QString fileName, char delimeter, bool firstRowHeader)
{
    stopNow = false;
    lastexporttype = "csv";

    LastSaveEndDate = QTime::currentTime();
    LastSaveDuration = QTime::currentTime();
    saveRowsDone = 0;
    exporting = true;
    exported = true;
    if(tbldata.size()>0 && tbldata[0].size()>0)
        saveRowSize = tbldata[0].size();
    else
    {
        LastSaveDuration.setHMS(0,0,0,0);
        exporting = false;
        lastExportSuccess = true;
        emit ExportedToCSV();
        return true;// no rows to export, so no point in saving
    }


    QFile fl((additionalSaveFileData + fileName));
    if(fl.open(QFile::OpenModeFlag::WriteOnly))
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

                if(stopNow)
                {
                    exporting = false;
                    lastExportSuccess = false;
                    LastSaveDuration = QTime::fromMSecsSinceStartOfDay(LastSaveDuration.msecsTo(QTime::currentTime()));
                    emit ExportedToCSV();
                    return false;
                }

                for(int i=0;i<tbldata.size();i++)
                {
                    QString str = tbldata[i][a];

                    if(str.contains(delimeter))
                    {
                        if(delimeter == ';')
                            str = str.replace(";",",");
                        else
                            str = str.replace(delimeter,";");
                    }
                    if(maxVarTypes[i] == 16)
                    {

                        str.resize(19);
                        str = str.replace("T"," ");
                        fl.write(str.toUtf8().constData());
                    }
                    else if(maxVarTypes[i] == 6)
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

                saveRowsDone++;
            }
    }
    else
    {
        qDebug() <<"failed to open file "<< (additionalSaveFileData + fileName);
        LastSaveDuration = QTime::fromMSecsSinceStartOfDay(LastSaveDuration.msecsTo(QTime::currentTime()));
        LastSaveEndDate = QTime::currentTime();
        exporting = false;
        lastExportSuccess = false;
        emit ExportedToCSV();
        return false;
    }
    fl.close();

    LastSaveDuration = QTime::fromMSecsSinceStartOfDay(LastSaveDuration.msecsTo(QTime::currentTime()));
    LastSaveEndDate = QTime::currentTime();
    exporting = false;
    lastExportSuccess = true;
    emit ExportedToCSV();
    return true;
}
bool TableData::ExportToExcel(QString fileName, int x_start,int x_end,int y_start,int y_end, bool firstRowHeader, QString sheetName, bool append)
{
    stopNow = false;
    if(append)
    {
        return AppendToExcel(fileName,sheetName);
    }
    exported = true;

    lastexporttype = "xlsx";
    LastSaveEndDate = QTime::currentTime();
    LastSaveDuration = QTime::currentTime();
    saveRowsDone = 0;
    exporting = true;
    if(tbldata.size()>0 && tbldata[0].size()>0)
        saveRowSize = tbldata[0].size();
    else
    {
        LastSaveDuration.setHMS(0,0,0,0);
        exporting = false;
        lastExportSuccess = false;
        emit ExportedToExcel();
        return true;// no rows to export, so no point in saving
    }

    if(tbldata.size()<=0)
    {
        exporting = false;
        lastExportSuccess = false;
        emit ExportedToExcel();
        return false;
    }
    bool diap = false;
    if(x_start != 0 || x_end != 0 || y_start != 0 || y_end != 0 )
    {// save into diapozon
        diap = true;
        qDebug() << "saving boundries "<< x_start <<  x_end << y_start << y_end;
    }
    // to load only if needed
    QString flopenname = "";
    if(sheetName!="" || diap)
    {
        flopenname = fileName;
    }

    QXlsx::Document xlsxR3(flopenname);

    if(sheetName!="" || diap)
    {
        xlsxR3.load();// load and clear sheet

        if(!append && !diap)
        {
            if(xlsxR3.sheetNames().contains(sheetName))
                xlsxR3.deleteSheet(sheetName);

            xlsxR3.addSheet(sheetName);
        }

        xlsxR3.selectSheet(sheetName);
    }
    if(append && y_start <=0)
        y_start=1;
    if(append)
    {
        while(!xlsxR3.read(y_start,1).isNull())
            y_start++;
        y_start-=2;
        qDebug() << y_start;
    }


    qDebug() << "saving " << fileName;
    int rowoffset = -y_start;
    int sheetnumber = 1;
    int column_offset = x_start;
    if(firstRowHeader && !append)
        for(int i=0;i<headers.size();i++)
        {
            QVariant var = headers[i];

            xlsxR3.write(1 + y_start,i+1 + x_start,var);
        }


    // for rows done approximation
    int _tmp_cell_count = 0;

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
            int column = i + 1 + column_offset;
            int _dt_valid_count = 0;
            for(int a=0;a<tbldata[i].size();a++)
            {
                if(stopNow)
                {
                    exporting = false;
                    lastExportSuccess = false;
                    LastSaveDuration = QTime::fromMSecsSinceStartOfDay(LastSaveDuration.msecsTo(QTime::currentTime()));
                    emit ExportedToExcel();
                    return false;
                }

                if(diap && a >= y_end - y_start + 1)
                    break;

                // to approximate row save count
                _tmp_cell_count++;
                if(_tmp_cell_count>=tbldata.size())
                {
                    _tmp_cell_count = 0;
                    saveRowsDone++;
                }

                int row = a + 2 - rowoffset - (!firstRowHeader);

                //6 = double, 10 = QString, 16 = DateTime
                if(maxVarTypes.size() == tbldata.size())
                {
                    if(maxVarTypes[i] == 10 || maxVarTypes[i] == QMetaType::QDateTime)
                    {
                        QString str = tbldata[i][a];
                        if(str.size() == 23 || str.size() == 19|| str.size() == 10)
                        {

                            //
                            //2025-01-01 10:10:10.123;

                            //if(dt.isValid())


                            if( (str.size() == 19 && str[0].isDigit() && str[1].isDigit() && str[2].isDigit() && str[3].isDigit() && !str[4].isDigit()//datetime
                                && str[5].isDigit() && str[6].isDigit() && !str[7].isDigit()
                                 && str[8].isDigit() && str[9].isDigit() && !str[10].isDigit()

                                 && str[11].isDigit() && str[12].isDigit() && !str[13].isDigit()
                                 && str[14].isDigit() && str[15].isDigit() && !str[16].isDigit()
                                && str[17].isDigit() && str[18].isDigit())
                                ||
                                (str.size() == 19 && str[0].isDigit() && str[1].isDigit() && str[2].isDigit() && str[3].isDigit() && !str[4].isDigit()//datetime
                                    && str[5].isDigit() && str[6].isDigit() && !str[7].isDigit()
                                    && str[8].isDigit() && str[9].isDigit() && !str[10].isDigit()

                                 && str[11].isDigit() && str[12].isDigit() && !str[13].isDigit()
                                 && str[14].isDigit() && str[15].isDigit() && !str[16].isDigit()
                                 && str[17].isDigit() && str[18].isDigit()
                                 && str[19]== '.' && str[20].isDigit()&& str[21].isDigit()&& str[22].isDigit())
                                ||
                                (str.size() == 10 && str[0].isDigit() && str[1].isDigit() && str[2].isDigit() && str[3].isDigit() && !str[4].isDigit()//datetime
                                    && str[5].isDigit() && str[6].isDigit() && !str[7].isDigit()
                                    && str[8].isDigit() && str[9].isDigit())
                                )
                            {

                                _dt_valid_count++;
                                const int XLSX_ROW_MAX    = 1048576;
                                const int XLSX_COLUMN_MAX = 16384;
                                if (!(row > XLSX_ROW_MAX || row < 1 || column > XLSX_COLUMN_MAX || column < 1))
                                {
                                    QString year="    ";year[0] = str[0]; year[1] = str[1]; year [2] = str[2]; year[3] = str[3];
                                    QString month = "  "; month[0] = str[5]; month[1] = str[6];
                                    QString day = "  "; day[0] = str[8]; day[1] = str[9];

                                    int i_year = year.toInt();
                                    int i_month = month.toInt();
                                    int i_day = day.toInt();
                                    double currentdt = (int(( 1461 * ( i_year + 4800 + int(( i_month - 14 ) / 12) ) ) / 4) +
                                                        int(( 367 * ( i_month - 2 - 12 * ( ( i_month - 14 ) / 12 ) ) ) / 12) -
                                                        int(( 3 * ( int(( i_year + 4900 + int(( i_month - 14 ) / 12) ) / 100) ) ) / 4) +
                                                        i_day - 2415019 - 32075);

                                    if(str.size() >= 19)
                                    {

                                        QString hour = str[11]; hour += str[12];
                                        QString minute = str[14]; minute += str[15];
                                        QString second = str[17]; second += str[18];
                                        currentdt += (second.toInt() * 1.0  +minute.toInt()*60.0 + hour.toInt()*3600.0)/86400.0;
                                    }
                                    double value = currentdt;

                                    xlsxR3.write(row,column,value);
                                }
                            }
                            else xlsxR3.write(row,column,tbldata[i][a]);
                        }
                        else
                            xlsxR3.write(row,column,tbldata[i][a]);
                    }
                    else if(maxVarTypes[i] == 6)
                    {

                        if(tbldata[i][a].startsWith('0') && !tbldata[i][a].startsWith("0."))
                        {
                            xlsxR3.write(row,column,tbldata[i][a]);
                        }
                        else
                        {

                            bool ok = false;
                            QVariant vardoubl = tbldata[i][a].toDouble(&ok);
                            if(ok)
                                xlsxR3.write(row,column,vardoubl);
                            else {
                                QVariant varint = tbldata[i][a].toLongLong(&ok);
                                if(ok)
                                    xlsxR3.write(row,column,varint);
                                else {
                                    xlsxR3.write(row,column,tbldata[i][a]);
                                }
                            }
                        }
                    }
                    else
                    {
                        QString str = tbldata[i][a];
                        if(str.size() == 23 || str.size() == 19|| str.size() == 10)
                        {

                                //
                                //2025-01-01 10:10:10.123;

                            //if(dt.isValid())


                                if( (str.size() == 19 && str[0].isDigit() && str[1].isDigit() && str[2].isDigit() && str[3].isDigit() && !str[4].isDigit()//datetime
                                     && str[5].isDigit() && str[6].isDigit() && !str[7].isDigit()
                                     && str[8].isDigit() && str[9].isDigit() && !str[10].isDigit()

                                     && str[11].isDigit() && str[12].isDigit() && !str[13].isDigit()
                                     && str[14].isDigit() && str[15].isDigit() && !str[16].isDigit()
                                     && str[17].isDigit() && str[18].isDigit())
                                    ||
                                    (str.size() == 19 && str[0].isDigit() && str[1].isDigit() && str[2].isDigit() && str[3].isDigit() && !str[4].isDigit()//datetime
                                     && str[5].isDigit() && str[6].isDigit() && !str[7].isDigit()
                                     && str[8].isDigit() && str[9].isDigit() && !str[10].isDigit()

                                     && str[11].isDigit() && str[12].isDigit() && !str[13].isDigit()
                                     && str[14].isDigit() && str[15].isDigit() && !str[16].isDigit()
                                     && str[17].isDigit() && str[18].isDigit()
                                     && str[19]== '.' && str[20].isDigit()&& str[21].isDigit()&& str[22].isDigit())
                                    ||
                                    (str.size() == 10 && str[0].isDigit() && str[1].isDigit() && str[2].isDigit() && str[3].isDigit() && !str[4].isDigit()//datetime
                                     && str[5].isDigit() && str[6].isDigit() && !str[7].isDigit()
                                     && str[8].isDigit() && str[9].isDigit())
                                    )
                            {

                                _dt_valid_count++;
                                const int XLSX_ROW_MAX    = 1048576;
                                const int XLSX_COLUMN_MAX = 16384;
                                if (!(row > XLSX_ROW_MAX || row < 1 || column > XLSX_COLUMN_MAX || column < 1))
                                {
                                    QString year="    ";year[0] = str[0]; year[1] = str[1]; year [2] = str[2]; year[3] = str[3];
                                    QString month = "  "; month[0] = str[5]; month[1] = str[6];
                                    QString day = "  "; day[0] = str[8]; day[1] = str[9];

                                    int i_year = year.toInt();
                                    int i_month = month.toInt();
                                    int i_day = day.toInt();
                                    double currentdt = (int(( 1461 * ( i_year + 4800 + int(( i_month - 14 ) / 12) ) ) / 4) +
                                                        int(( 367 * ( i_month - 2 - 12 * ( ( i_month - 14 ) / 12 ) ) ) / 12) -
                                                        int(( 3 * ( int(( i_year + 4900 + int(( i_month - 14 ) / 12) ) / 100) ) ) / 4) +
                                                        i_day - 2415019 - 32075);

                                    if(str.size() >= 19)
                                    {

                                        QString hour = str[11]; hour += str[12];
                                        QString minute = str[14]; minute += str[15];
                                        QString second = str[17]; second += str[18];
                                        currentdt += (second.toInt() * 1.0  +minute.toInt()*60.0 + hour.toInt()*3600.0)/86400.0;
                                    }
                                    double value = currentdt;

                                    xlsxR3.write(row,column,value);
                                }
                            }
                            else xlsxR3.write(row,column,tbldata[i][a]);
                        }
                        else
                            xlsxR3.write(row,column,tbldata[i][a]);
                    }
                }
                else
                {
                    QString str = tbldata[i][a];
                    if(str.size() == 23 || str.size() == 19|| str.size() == 10)
                    {

                            //
                            //2025-01-01 10:10:10.123;

                        //if(dt.isValid())


                            if( (str.size() == 19 && str[0].isDigit() && str[1].isDigit() && str[2].isDigit() && str[3].isDigit() && !str[4].isDigit()//datetime
                                 && str[5].isDigit() && str[6].isDigit() && !str[7].isDigit()
                                 && str[8].isDigit() && str[9].isDigit() && !str[10].isDigit()

                                 && str[11].isDigit() && str[12].isDigit() && !str[13].isDigit()
                                 && str[14].isDigit() && str[15].isDigit() && !str[16].isDigit()
                                 && str[17].isDigit() && str[18].isDigit())
                                ||
                                (str.size() == 19 && str[0].isDigit() && str[1].isDigit() && str[2].isDigit() && str[3].isDigit() && !str[4].isDigit()//datetime
                                 && str[5].isDigit() && str[6].isDigit() && !str[7].isDigit()
                                 && str[8].isDigit() && str[9].isDigit() && !str[10].isDigit()

                                 && str[11].isDigit() && str[12].isDigit() && !str[13].isDigit()
                                 && str[14].isDigit() && str[15].isDigit() && !str[16].isDigit()
                                 && str[17].isDigit() && str[18].isDigit()
                                 && str[19]== '.' && str[20].isDigit()&& str[21].isDigit()&& str[22].isDigit())
                                ||
                                (str.size() == 10 && str[0].isDigit() && str[1].isDigit() && str[2].isDigit() && str[3].isDigit() && !str[4].isDigit()//datetime
                                 && str[5].isDigit() && str[6].isDigit() && !str[7].isDigit()
                                 && str[8].isDigit() && str[9].isDigit())
                                )
                        {

                            _dt_valid_count++;
                            const int XLSX_ROW_MAX    = 1048576;
                            const int XLSX_COLUMN_MAX = 16384;
                            if (!(row > XLSX_ROW_MAX || row < 1 || column > XLSX_COLUMN_MAX || column < 1))
                            {
                                QString year="    ";year[0] = str[0]; year[1] = str[1]; year [2] = str[2]; year[3] = str[3];
                                QString month = "  "; month[0] = str[5]; month[1] = str[6];
                                QString day = "  "; day[0] = str[8]; day[1] = str[9];

                                int i_year = year.toInt();
                                int i_month = month.toInt();
                                int i_day = day.toInt();
                                double currentdt = (int(( 1461 * ( i_year + 4800 + int(( i_month - 14 ) / 12) ) ) / 4) +
                                                    int(( 367 * ( i_month - 2 - 12 * ( ( i_month - 14 ) / 12 ) ) ) / 12) -
                                                    int(( 3 * ( int(( i_year + 4900 + int(( i_month - 14 ) / 12) ) / 100) ) ) / 4) +
                                                    i_day - 2415019 - 32075);

                                if(str.size() >= 19)
                                {

                                    QString hour = str[11]; hour += str[12];
                                    QString minute = str[14]; minute += str[15];
                                    QString second = str[17]; second += str[18];
                                    currentdt += (second.toInt() * 1.0  +minute.toInt()*60.0 + hour.toInt()*3600.0)/86400.0;
                                }
                                double value = currentdt;

                                xlsxR3.write(row,column,value);
                            }
                        }
                        else xlsxR3.write(row,column,tbldata[i][a]);
                    }
                    else
                        xlsxR3.write(row,column,tbldata[i][a]);
                }

            }

            if(_dt_valid_count > tbldata[i].size()*0.5)
            {
                QXlsx::Format fmt = xlsxR3.cellAt(1,column)->format();
                fmt.setNumberFormat(xlsxR3.workbook()->defaultDateFormat());
                xlsxR3.setColumnFormat(column,fmt);
                xlsxR3.setColumnWidth(column,20);
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
                int column = i + 1 + column_offset;
                int _dt_valid_count = 0;
                for(int a=start ;a<end ;a++)
                {
                    if(stopNow)
                    {
                        exporting = false;
                        lastExportSuccess = false;
                        LastSaveDuration = QTime::fromMSecsSinceStartOfDay(LastSaveDuration.msecsTo(QTime::currentTime()));
                        emit ExportedToExcel();
                        return false;
                    }

                    // to approximate row save count
                    _tmp_cell_count++;
                    if(_tmp_cell_count>=tbldata.size())
                    {
                        _tmp_cell_count = 0;
                        saveRowsDone++;
                    }
                    int row = a + 2 - start - (!firstRowHeader);

                    //6 = double, 10 = QString, 16 = DateTime
                    if(maxVarTypes.size() == tbldata.size())
                    {
                        if(maxVarTypes[i] == 10 || maxVarTypes[i] == 16)
                        {
                            QString str = tbldata[i][a];
                            if(str.size() == 23 || str.size() == 19|| str.size() == 10)
                            {

                                    //
                                    //2025-01-01 10:10:10.123;

                                //if(dt.isValid())


                                    if( (str.size() == 19 && str[0].isDigit() && str[1].isDigit() && str[2].isDigit() && str[3].isDigit() && !str[4].isDigit()//datetime
                                         && str[5].isDigit() && str[6].isDigit() && !str[7].isDigit()
                                         && str[8].isDigit() && str[9].isDigit() && !str[10].isDigit()

                                         && str[11].isDigit() && str[12].isDigit() && !str[13].isDigit()
                                         && str[14].isDigit() && str[15].isDigit() && !str[16].isDigit()
                                         && str[17].isDigit() && str[18].isDigit())
                                        ||
                                        (str.size() == 19 && str[0].isDigit() && str[1].isDigit() && str[2].isDigit() && str[3].isDigit() && !str[4].isDigit()//datetime
                                         && str[5].isDigit() && str[6].isDigit() && !str[7].isDigit()
                                         && str[8].isDigit() && str[9].isDigit() && !str[10].isDigit()

                                         && str[11].isDigit() && str[12].isDigit() && !str[13].isDigit()
                                         && str[14].isDigit() && str[15].isDigit() && !str[16].isDigit()
                                         && str[17].isDigit() && str[18].isDigit()
                                         && str[19]== '.' && str[20].isDigit()&& str[21].isDigit()&& str[22].isDigit())
                                        ||
                                        (str.size() == 10 && str[0].isDigit() && str[1].isDigit() && str[2].isDigit() && str[3].isDigit() && !str[4].isDigit()//datetime
                                         && str[5].isDigit() && str[6].isDigit() && !str[7].isDigit()
                                         && str[8].isDigit() && str[9].isDigit())
                                        )
                                {

                                    _dt_valid_count++;
                                    const int XLSX_ROW_MAX    = 1048576;
                                    const int XLSX_COLUMN_MAX = 16384;
                                    if (!(row > XLSX_ROW_MAX || row < 1 || column > XLSX_COLUMN_MAX || column < 1))
                                    {
                                        QString year="    ";year[0] = str[0]; year[1] = str[1]; year [2] = str[2]; year[3] = str[3];
                                        QString month = "  "; month[0] = str[5]; month[1] = str[6];
                                        QString day = "  "; day[0] = str[8]; day[1] = str[9];

                                        int i_year = year.toInt();
                                        int i_month = month.toInt();
                                        int i_day = day.toInt();
                                        double currentdt = (int(( 1461 * ( i_year + 4800 + int(( i_month - 14 ) / 12) ) ) / 4) +
                                                            int(( 367 * ( i_month - 2 - 12 * ( ( i_month - 14 ) / 12 ) ) ) / 12) -
                                                            int(( 3 * ( int(( i_year + 4900 + int(( i_month - 14 ) / 12) ) / 100) ) ) / 4) +
                                                            i_day - 2415019 - 32075);

                                        if(str.size() >= 19)
                                        {

                                            QString hour = str[11]; hour += str[12];
                                            QString minute = str[14]; minute += str[15];
                                            QString second = str[17]; second += str[18];
                                            currentdt += (second.toInt() * 1.0  +minute.toInt()*60.0 + hour.toInt()*3600.0)/86400.0;
                                        }
                                        double value = currentdt;

                                        xlsxR3.write(row,column,value);
                                    }
                                }
                                else xlsxR3.write(row,column,tbldata[i][a]);
                            }
                            else
                                xlsxR3.write(row,column,tbldata[i][a]);
                        }
                        else if(maxVarTypes[i] == 6)
                        {
                            if(tbldata[i][a].startsWith('0') && !tbldata[i][a].startsWith("0."))
                            {
                                xlsxR3.write(row,column,tbldata[i][a]);
                            }
                            else
                            {

                                bool ok = false;
                                QVariant vardoubl = tbldata[i][a].toDouble(&ok);
                                if(ok)
                                    xlsxR3.write(row,column,vardoubl);
                                else {
                                    QVariant varint = tbldata[i][a].toLongLong(&ok);
                                    if(ok)
                                        xlsxR3.write(row,column,varint);
                                    else {
                                        xlsxR3.write(row,column,tbldata[i][a]);
                                    }
                                }
                            }
                        }
                        else
                        {
                            QString str = tbldata[i][a];
                            if(str.size() == 23 || str.size() == 19|| str.size() == 10)
                            {

                                    //
                                    //2025-01-01 10:10:10.123;

                                //if(dt.isValid())


                                    if( (str.size() == 19 && str[0].isDigit() && str[1].isDigit() && str[2].isDigit() && str[3].isDigit() && !str[4].isDigit()//datetime
                                         && str[5].isDigit() && str[6].isDigit() && !str[7].isDigit()
                                         && str[8].isDigit() && str[9].isDigit() && !str[10].isDigit()

                                         && str[11].isDigit() && str[12].isDigit() && !str[13].isDigit()
                                         && str[14].isDigit() && str[15].isDigit() && !str[16].isDigit()
                                         && str[17].isDigit() && str[18].isDigit())
                                        ||
                                        (str.size() == 19 && str[0].isDigit() && str[1].isDigit() && str[2].isDigit() && str[3].isDigit() && !str[4].isDigit()//datetime
                                         && str[5].isDigit() && str[6].isDigit() && !str[7].isDigit()
                                         && str[8].isDigit() && str[9].isDigit() && !str[10].isDigit()

                                         && str[11].isDigit() && str[12].isDigit() && !str[13].isDigit()
                                         && str[14].isDigit() && str[15].isDigit() && !str[16].isDigit()
                                         && str[17].isDigit() && str[18].isDigit()
                                         && str[19]== '.' && str[20].isDigit()&& str[21].isDigit()&& str[22].isDigit())
                                        ||
                                        (str.size() == 10 && str[0].isDigit() && str[1].isDigit() && str[2].isDigit() && str[3].isDigit() && !str[4].isDigit()//datetime
                                         && str[5].isDigit() && str[6].isDigit() && !str[7].isDigit()
                                         && str[8].isDigit() && str[9].isDigit())
                                        )
                                {

                                    _dt_valid_count++;
                                    const int XLSX_ROW_MAX    = 1048576;
                                    const int XLSX_COLUMN_MAX = 16384;
                                    if (!(row > XLSX_ROW_MAX || row < 1 || column > XLSX_COLUMN_MAX || column < 1))
                                    {
                                        QString year="    ";year[0] = str[0]; year[1] = str[1]; year [2] = str[2]; year[3] = str[3];
                                        QString month = "  "; month[0] = str[5]; month[1] = str[6];
                                        QString day = "  "; day[0] = str[8]; day[1] = str[9];

                                        int i_year = year.toInt();
                                        int i_month = month.toInt();
                                        int i_day = day.toInt();
                                        double currentdt = (int(( 1461 * ( i_year + 4800 + int(( i_month - 14 ) / 12) ) ) / 4) +
                                                            int(( 367 * ( i_month - 2 - 12 * ( ( i_month - 14 ) / 12 ) ) ) / 12) -
                                                            int(( 3 * ( int(( i_year + 4900 + int(( i_month - 14 ) / 12) ) / 100) ) ) / 4) +
                                                            i_day - 2415019 - 32075);

                                        if(str.size() >= 19)
                                        {

                                            QString hour = str[11]; hour += str[12];
                                            QString minute = str[14]; minute += str[15];
                                            QString second = str[17]; second += str[18];
                                            currentdt += (second.toInt() * 1.0  +minute.toInt()*60.0 + hour.toInt()*3600.0)/86400.0;
                                        }
                                        double value = currentdt;

                                        xlsxR3.write(row,column,value);
                                    }
                                }
                                else xlsxR3.write(row,column,tbldata[i][a]);
                            }
                            else
                                xlsxR3.write(row,column,tbldata[i][a]);
                        }
                    }
                    else
                    {
                        QString str = tbldata[i][a];
                        if(str.size() == 23 || str.size() == 19|| str.size() == 10)
                        {

                                //
                                //2025-01-01 10:10:10.123;

                            //if(dt.isValid())


                                if( (str.size() == 19 && str[0].isDigit() && str[1].isDigit() && str[2].isDigit() && str[3].isDigit() && !str[4].isDigit()//datetime
                                     && str[5].isDigit() && str[6].isDigit() && !str[7].isDigit()
                                     && str[8].isDigit() && str[9].isDigit() && !str[10].isDigit()

                                     && str[11].isDigit() && str[12].isDigit() && !str[13].isDigit()
                                     && str[14].isDigit() && str[15].isDigit() && !str[16].isDigit()
                                     && str[17].isDigit() && str[18].isDigit())
                                    ||
                                    (str.size() == 19 && str[0].isDigit() && str[1].isDigit() && str[2].isDigit() && str[3].isDigit() && !str[4].isDigit()//datetime
                                     && str[5].isDigit() && str[6].isDigit() && !str[7].isDigit()
                                     && str[8].isDigit() && str[9].isDigit() && !str[10].isDigit()

                                     && str[11].isDigit() && str[12].isDigit() && !str[13].isDigit()
                                     && str[14].isDigit() && str[15].isDigit() && !str[16].isDigit()
                                     && str[17].isDigit() && str[18].isDigit()
                                     && str[19]== '.' && str[20].isDigit()&& str[21].isDigit()&& str[22].isDigit())
                                    ||
                                    (str.size() == 10 && str[0].isDigit() && str[1].isDigit() && str[2].isDigit() && str[3].isDigit() && !str[4].isDigit()//datetime
                                     && str[5].isDigit() && str[6].isDigit() && !str[7].isDigit()
                                     && str[8].isDigit() && str[9].isDigit())
                                    )
                            {

                                _dt_valid_count++;
                                const int XLSX_ROW_MAX    = 1048576;
                                const int XLSX_COLUMN_MAX = 16384;
                                if (!(row > XLSX_ROW_MAX || row < 1 || column > XLSX_COLUMN_MAX || column < 1))
                                {
                                    QString year="    ";year[0] = str[0]; year[1] = str[1]; year [2] = str[2]; year[3] = str[3];
                                    QString month = "  "; month[0] = str[5]; month[1] = str[6];
                                    QString day = "  "; day[0] = str[8]; day[1] = str[9];

                                    int i_year = year.toInt();
                                    int i_month = month.toInt();
                                    int i_day = day.toInt();
                                    double currentdt = (int(( 1461 * ( i_year + 4800 + int(( i_month - 14 ) / 12) ) ) / 4) +
                                                        int(( 367 * ( i_month - 2 - 12 * ( ( i_month - 14 ) / 12 ) ) ) / 12) -
                                                        int(( 3 * ( int(( i_year + 4900 + int(( i_month - 14 ) / 12) ) / 100) ) ) / 4) +
                                                        i_day - 2415019 - 32075);

                                    if(str.size() >= 19)
                                    {

                                        QString hour = str[11]; hour += str[12];
                                        QString minute = str[14]; minute += str[15];
                                        QString second = str[17]; second += str[18];
                                        currentdt += (second.toInt() * 1.0  +minute.toInt()*60.0 + hour.toInt()*3600.0)/86400.0;
                                    }
                                    double value = currentdt;

                                    xlsxR3.write(row,column,value);
                                }
                            }
                            else xlsxR3.write(row,column,tbldata[i][a]);
                        }
                        else
                            xlsxR3.write(row,column,tbldata[i][a]);
                    }
                }

                if(_dt_valid_count > tbldata[i].size()*0.5)
                {
                    QXlsx::Format fmt = xlsxR3.cellAt(1,column)->format();
                    fmt.setNumberFormat(xlsxR3.workbook()->defaultDateFormat());
                    xlsxR3.setColumnFormat(column,fmt);
                    xlsxR3.setColumnWidth(column,20);
                }
            }

            if(end >= tbldata[0].size())
                break;
            sheetnumber++;

            start = end;
            end += 1'000'000;
            if(end > tbldata[0].size())
                end = tbldata[0].size();

            QString sheetname = sheetName;
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
        LastSaveDuration = QTime::fromMSecsSinceStartOfDay(LastSaveDuration.msecsTo(QTime::currentTime()));
        LastSaveEndDate = QTime::currentTime();
        exporting = false;
        emit ExportedToExcel();
        lastExportSuccess = true;
        return true;
    }
    else
    {
        qDebug()<<"failed to save, file probably opened.";
        LastSaveDuration = QTime::fromMSecsSinceStartOfDay(LastSaveDuration.msecsTo(QTime::currentTime()));
        LastSaveEndDate = QTime::currentTime();
        exporting = false;
        emit ExportedToExcel();
        lastExportSuccess = false;
        return false;
    }
}
bool TableData::ExportToSQLiteTable(QString tableName)
{
    stopNow = false;
    lastexporttype = "Local";
    LastSaveEndDate = QTime::currentTime();
    LastSaveDuration = QTime::currentTime();
    saveRowsDone = 0;
    exporting = true;
    exported = true;
    if(tbldata.size()>0 && tbldata[0].size()>0)
        saveRowSize = tbldata[0].size();
    else
    {
        LastSaveDuration.setHMS(0,0,0,0);
        exporting = false;
        lastExportSuccess = true;
        emit ExportedToSQLiteTable();
        return true;// no rows to export, so no point in saving
    }
    userDS.Load(documentsDir +"/userdata.txt");
    DatabaseConnection dc;
    dc.nodebug = true;
    dc.rawquery = true;
    dc.disableSaveToUserDS = true;
    QString driver = userDS.data["UserTheme"]["db_drv_Save_table_driver"];
    QString conection = userDS.data["UserTheme"]["db_drv_Save_table_Connection"];
    qDebug() <<driver << "  " << conection;
    conection.replace("documentsDir",documentsDir);

    dc.Create(driver.trimmed(), conection.trimmed());




    QString SQLITE_sql = "Drop table ";

    if(!dc.sqlite)
        SQLITE_sql = "Drop table if exists ";

    if(!isWord(tableName) ||tableName.contains(".")|| tableName.contains(" "))
        SQLITE_sql += "\"";
    SQLITE_sql += tableName;
    if(!isWord(tableName) ||tableName.contains(".")|| tableName.contains(" "))
        SQLITE_sql += "\"";
    SQLITE_sql += ";";
    if(dc.execSql(SQLITE_sql))
        qDebug()<< "Dropped sqlite table";

    SQLITE_sql = "Create table ";
    if(!dc.sqlite)
        SQLITE_sql = "Create table if not exists ";

    if(!isWord(tableName) ||tableName.contains(".")|| tableName.contains(" "))
        SQLITE_sql += "\"";
    SQLITE_sql += tableName;

    if(!isWord(tableName) ||tableName.contains(".")|| tableName.contains(" "))
        SQLITE_sql += "\"";


    SQLITE_sql += " ( ";
    for(int i=0;i<headers.size();i++)
    {
        bool need_quotation = false;
        if(!isWord(headers[i]) || headers[i].contains(".")|| headers[i].contains(" "))
            need_quotation = true;

        if(need_quotation) SQLITE_sql += "\"";
        SQLITE_sql += headers[i].trimmed();
        if(need_quotation) SQLITE_sql += "\"";

        int doublecnt = 0;
        int othercnt = 0;

        bool typecountHasString = false;

        if(typecount.size() > i && typecount[i].size() >10 && typecount[i][10] >0)
            typecountHasString = true;

        if(!dc.postgre || typecountHasString)
        {
            SQLITE_sql += " text ";

        }
        else
        {// numeric, timestamp, text
            if(maxVarTypes[i] == 6)
                SQLITE_sql += " numeric ";
            else if (maxVarTypes[i] == 16)
                SQLITE_sql += " timestamp ";
            else
                SQLITE_sql += " text ";
        }

        if(i+1<headers.size())
            SQLITE_sql += ", ";
    }
    SQLITE_sql += " )";
    qDebug()<< SQLITE_sql;

    if(dc.execSql(SQLITE_sql))
        qDebug()<< "Created sqlite table";
    else
    {
        qDebug()<< "Failed to create sqlite table";
        LastSaveDuration = QTime::fromMSecsSinceStartOfDay(LastSaveDuration.msecsTo(QTime::currentTime()));
        LastSaveEndDate = QTime::currentTime();
        exporting = false;
        lastExportSuccess = false;
        emit ExportedToSQLiteTable();
        return false;
    }


    SQLITE_sql = "create table if not exists user_table_desc ( "
                 " exec_table_name text, "
                 " exec_driver text, "
                 " exec_database text, "
                 " exec_user_name text, "
                 " exec_workspace_name text, "
                 " cre_date timestamp, "
                 " exec_sql text "
                 ")";

    if(dc.sqlite)
        SQLITE_sql = "create table if not exists user_table_desc ( "
                 " exec_table_name text, "
                 " exec_driver text, "
                 " exec_database text, "
                 " exec_user_name text, "
                 " exec_workspace_name text, "
                 " cre_date text, "
                 " exec_sql text "
                 ")";

    if(dc.execSql(SQLITE_sql))
        qDebug()<< "Created local table description";
    else qDebug() << "FAIL: Created local table description";

    if(!dc.execSql("select * from  user_table_desc where exec_table_name = '" + tableName + "'"))
        qDebug()<< "Can't load table description";
    else
    {
        if(dc.data.tbldata.size() >0 && dc.data.tbldata[0].size()<=0)
        {
            SQLITE_sql = "Insert into user_table_desc values( '";
            SQLITE_sql += tableName;
            SQLITE_sql += "', '";

            SQLITE_sql += LastDriver;
            SQLITE_sql += "', '";

            SQLITE_sql += LastDatabase;
            SQLITE_sql += "', '";

            SQLITE_sql += LastUser;
            SQLITE_sql += "', '";
            SQLITE_sql += loadWind->LastWorkspaceName;
            SQLITE_sql += "', ";
            if(dc.postgre)
                SQLITE_sql += "clock_timestamp ( )";
            if(dc.sqlite)
                SQLITE_sql += "datetime ('now')";
            SQLITE_sql += ", '";
            SQLITE_sql += sqlCode.replace("'","''");
            SQLITE_sql += "')";

            if(dc.execSql(SQLITE_sql))
                qDebug()<< "inserted local table description";
            else qDebug() << "FAIL: inserted local table description";
        }
        else
        {
            SQLITE_sql = "update user_table_desc set ";

            SQLITE_sql += " exec_driver = '" + LastDriver + "'";
            SQLITE_sql += " ,exec_database = '" + LastDatabase + "'";
            SQLITE_sql += " ,exec_user_name = '" + LastUser +"'";
            SQLITE_sql += " ,exec_workspace_name = '" + loadWind->LastWorkspaceName + "' ";
            SQLITE_sql += ", cre_date = ";
            if(dc.postgre)
                SQLITE_sql += " clock_timestamp ( )";
            if(dc.sqlite)
                SQLITE_sql += " datetime ('now')";
            SQLITE_sql += " ";

            SQLITE_sql += " ,exec_sql = '" +  sqlCode.replace("'","''") + "'";
            SQLITE_sql += " where exec_table_name = '" + tableName + "'";

            dc.nodebug = false;
            if(dc.execSql(SQLITE_sql))
                qDebug()<< "inserted local table description";
            else qDebug() << "FAIL: inserted local table description ";
            dc.nodebug = true;

        }
    }


    SQLITE_sql = "Insert into ";
    if(!isWord(tableName) ||tableName.contains(".")|| tableName.contains(" "))
        SQLITE_sql += "\"";
    SQLITE_sql += tableName;
    if(!isWord(tableName) ||tableName.contains(".")|| tableName.contains(" "))
        SQLITE_sql += "\"";
    SQLITE_sql += " values ";

    bool firstVal = true;
    qDebug() << SQLITE_sql;
    int lasti=0;
    for(int i=0;i<tbldata[0].size();i++)
    {
        saveRowsDone++;
        if(!firstVal)
            SQLITE_sql += ",";
        if(firstVal)
                firstVal=false;
        SQLITE_sql += " (";

        //SQLITE_sql += " union all Select ";
        bool first = true;
        for(int a=0;a<tbldata.size();a++)
        {
            if(stopNow)
            {
                exporting = false;
                lastExportSuccess = false;
                LastSaveDuration = QTime::fromMSecsSinceStartOfDay(LastSaveDuration.msecsTo(QTime::currentTime()));
                emit ExportedToSQLiteTable();
                return false;
            }

            if(!first)
                SQLITE_sql += ",";
            first = false;
            int row =i+2;
            int column =a+1;

            //QVariant var = fixQVariantTypeFormat(tbldata[a][i]);


            if(dc.postgre  && !(typecount.size() > i && typecount[i].size() >10 && typecount[i][10] >0))
            {
                //fixQStringType_lasttype

                if(tbldata[a][i].size() >0)
                {
                    if(maxVarTypes[a] != 6  || (typecount[a].size()>10 && typecount[a][10]>0))
                        SQLITE_sql += " '";
                    SQLITE_sql += tbldata[a][i].replace("'","''");
                    if(maxVarTypes[a] != 6  || (typecount[a].size()>10 && typecount[a][10]>0))
                        SQLITE_sql += "' ";
                }
                else
                    SQLITE_sql += "null";

            }
            else
            {
                if(tbldata[a][i].size() >0)
                {
                    SQLITE_sql += " '";

                    SQLITE_sql += tbldata[a][i].replace("'","''");

                    SQLITE_sql += "' ";
                }
                else
                    SQLITE_sql += "null";

            }
        }
        SQLITE_sql += " )";
        if(i - lasti > 400)
        {
            firstVal=true;
            lasti = i;
            if(!dc.execSql(SQLITE_sql))
            {
                qDebug()<< "Failed to save to LOCAL: " << SQLITE_sql ;
                LastSaveDuration = QTime::fromMSecsSinceStartOfDay(LastSaveDuration.msecsTo(QTime::currentTime()));
                LastSaveEndDate = QTime::currentTime();
                exporting = false;
                lastExportSuccess = false;
                emit ExportedToSQLiteTable();
                return false;
            }
            SQLITE_sql = "Insert into ";
            if(!isWord(tableName) ||tableName.contains(".")|| tableName.contains(" "))
                SQLITE_sql += "\"";
            SQLITE_sql += tableName;
            if(!isWord(tableName) ||tableName.contains(".")|| tableName.contains(" "))
                SQLITE_sql += "\"";
            SQLITE_sql += " values ";

        }
    }
    if(!dc.execSql(SQLITE_sql))
        qDebug()<< "Failed to save to LOCAL: " << SQLITE_sql ;
    LastSaveDuration = QTime::fromMSecsSinceStartOfDay(LastSaveDuration.msecsTo(QTime::currentTime()));
    LastSaveEndDate = QTime::currentTime();
    exporting = false;
    lastExportSuccess = true;
    emit ExportedToSQLiteTable();
    return true;
}


bool TableData::AppendToCSV(QString fileName, char delimeter)
{
    stopNow = false;
    QFile fl((additionalSaveFileData + fileName));
    ;
    if(fl.open(QFile::OpenModeFlag::Append))
    {
        if(tbldata.size() > 0 && tbldata[0].size() > 0)
            for(int a=0;a<tbldata[0].size();a++)
            {
                for(int i=0;i<tbldata.size();i++)
                {
                    QString str = tbldata[i][a];

                    if(str.contains(delimeter))
                    {
                        if(delimeter == ';')
                            str = str.replace(";",",");
                        else
                            str = str.replace(delimeter,";");
                    }
                    QVariant var = fixQVariantTypeFormat(tbldata[i][a]);
                    if(var.typeId() == 16)
                    {

                        str.resize(19);
                        str = str.replace("T"," ");
                        fl.write(str.toUtf8().constData());
                    }
                    else if(var.typeId() == 6)
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
bool TableData::AppendToExcel(QString fileName, QString SheetName )
{
    stopNow = false;

    if(SheetName== "")
        SheetName = "Sheet1";
    DatabaseConnection dc;
    dc.Create("QODBC_Excel",fileName);
    dc.nodebug = true;
    dc.rawquery = true;
    dc.disableSaveToUserDS = true;
    QString SQLITE_sql = "";


    if(tbldata.size() <=0)
        return true;



    qDebug() << SQLITE_sql;
    int lasti=0;
    for(int i=0;i<tbldata[0].size();i++)
    {

        SQLITE_sql = "Insert into ";
        SQLITE_sql +=  "[" + SheetName + "$" + "]";
        SQLITE_sql += " values ";
        SQLITE_sql += " (";

        //SQLITE_sql += " union all Select ";
        bool first = true;
        for(int a=0;a<tbldata.size();a++)
        {
            if(!first)
                SQLITE_sql += ",";
            first = false;

            if(!(typecount.size() > i && typecount[i].size() >10 && typecount[i][10] >0))
            {
                //fixQStringType_lasttype

                if(tbldata[a][i].size() >0)
                {

                    bool ok = false;
                    tbldata[a][i].toDouble(&ok);
                    if(maxVarTypes[a] != 6 && !ok)
                        SQLITE_sql += " '";
                    SQLITE_sql += tbldata[a][i];
                    if(maxVarTypes[a] != 6 && !ok)
                        SQLITE_sql += "' ";
                }
                else
                    SQLITE_sql += "null";

            }
            else if(tbldata[a][i].size() >0)
            {

                bool ok = false;
                tbldata[a][i].toDouble(&ok);
                if(!ok)
                    SQLITE_sql += " '";

                SQLITE_sql += tbldata[a][i];

                if(!ok)
                    SQLITE_sql += "' ";
            }
            else
                SQLITE_sql += "null";


        }
        SQLITE_sql += " )";
        if(!dc.execSql(SQLITE_sql))
        {

            qDebug()<< "Failed to save to Excel: " << SQLITE_sql << dc.db.lastError().text();
        }
    }
    return true;
}


// Trash for QML, probably gonna be compleatly anused

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
