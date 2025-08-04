#ifndef TESTING_H
#define TESTING_H


#include <QApplication>
#include <QCoreApplication>
#include "Patterns.h"
#include "loaderwidnow.h"
#include "tabledata.h"



int xlsxReadWrite()
{

    TableData td;

    QStringList headers;
    headers << "int";
    headers << "doable";
    headers << "string";
    headers << "funnystring";
    headers << "datetime";
    td.headers = headers;
    td.tbldata.resize(headers.size());


    td.tbldata[0].push_back(QVariant(123).toString());
    td.tbldata[1].push_back(QVariant(123.123).toString());
    td.tbldata[2].push_back("123");
    td.tbldata[3].push_back("0123");
    td.tbldata[4].push_back("2025-01-01 10:11:12");



    td.FixTypeInfo();


    td.ExportToExcel("test.xlsx");

    TableData td2;
    td2.ImportFromExcel("test.xlsx");

    if( td2.tbldata[0] == td.tbldata[0] &&
        td2.tbldata[1] == td.tbldata[1] &&
        td2.tbldata[2] == td.tbldata[2] &&
        td2.tbldata[3] == td.tbldata[3] &&
        td2.tbldata[4] == td.tbldata[4])
        return 0;

    return 1;
}

int csvReadWrite()
{

    TableData td;

    QStringList headers;
    headers << "int";
    headers << "doable";
    headers << "string";
    headers << "funnystring";
    headers << "datetime";
    td.headers = headers;
    td.tbldata.resize(headers.size());


    td.tbldata[0].push_back(QVariant(123).toString());
    td.tbldata[1].push_back(QVariant(123.123).toString());
    td.tbldata[2].push_back("123");
    td.tbldata[3].push_back("0123");
    td.tbldata[4].push_back("2025-01-01 10:11:12");



    td.FixTypeInfo();


    td.ExportToCSV("test.csv",';');

    TableData td2;
    td2.ImportFromCSV("test.csv",';');

    if( td2.tbldata[0] == td.tbldata[0] &&
        td2.tbldata[1] == td.tbldata[1] &&
        td2.tbldata[2] == td.tbldata[2] &&
        td2.tbldata[3] == td.tbldata[3] &&
        td2.tbldata[4] == td.tbldata[4])
        return 0;

    return 1;
}
int LocalDBReadWrite()
{

    TableData td;

    QStringList headers;
    headers << "int";
    headers << "doable";
    headers << "string";
    headers << "funnystring";
    headers << "datetime";
    td.headers = headers;
    td.tbldata.resize(headers.size());


    td.tbldata[0].push_back(QVariant(123).toString());
    td.tbldata[1].push_back(QVariant(123.123).toString());
    td.tbldata[2].push_back("123");
    td.tbldata[3].push_back("0123");
    td.tbldata[4].push_back("2025-01-01 10:11:12");



    td.FixTypeInfo();


    td.ExportToSQLiteTable("___test___LocalDBReadWrite_");

    DatabaseConnection dc;
    dc.Create("LOCAL","LOCAL");
    dc.execSql("select * from ___test___LocalDBReadWrite_");

    if( dc.data.tbldata[0] == td.tbldata[0] &&
        dc.data.tbldata[1] == td.tbldata[1] &&
        dc.data.tbldata[2] == td.tbldata[2] &&
        dc.data.tbldata[3] == td.tbldata[3] &&
        dc.data.tbldata[4] == td.tbldata[4])
        return 0;

    return 1;
}

int testFunc(QString test_case)
{
    if(test_case == "fail")
        return 0;
    if(test_case == "xlsxReadWrite")
        return xlsxReadWrite();
    if(test_case == "csvReadWrite")
        return csvReadWrite();
    if(test_case == "LocalDBReadWrite")
        return LocalDBReadWrite();
    return 1;
}

#endif // TESTING_H
