#ifndef TESTING_H
#define TESTING_H


#include <QApplication>
#include <QCoreApplication>
#include "Patterns.h"
#include "loaderwidnow.h"
#include "tabledata.h"

/*
TODO:
add test for each sql subFunction, formating,

*/

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
    dc.disableSaveToUserDS = true;
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

int CycleTest()
{
    QString selectStmt = "QueryForLoop { --{iterator} {01} {07}\n"
                         "    select iterator as \"col1\", 'aiterator' as \"col2\"\n"
                         "}\n";


    DatabaseConnection dc;
    dc.disableSaveToUserDS = true;
    dc.Create("LOCAL","LOCAL");
    dc.execSql(selectStmt );

    if( dc.data.tbldata[0][0] == "1" &&
        dc.data.tbldata[1][0] == "a01" &&
        dc.data.tbldata[0][1] == "2" &&
        dc.data.tbldata[1][1] == "a02" &&
        dc.data.tbldata[0][2] == "3" &&
        dc.data.tbldata[1][2] == "a03" &&
        dc.data.tbldata[0][3] == "4" &&
        dc.data.tbldata[1][3] == "a04" &&
        dc.data.tbldata[0][4] == "5" &&
        dc.data.tbldata[1][4] == "a05" &&
        dc.data.tbldata[0][5] == "6" &&
        dc.data.tbldata[1][5] == "a06" &&
        dc.data.tbldata[0][6] == "7" &&
        dc.data.tbldata[1][6] == "a07" )
        return 0;
    return 1;
}


int SubexecTest()
{
    QString selectStmt = "SubexecToLocalDBTable { {LOCAL} {LOCAL} {__test_tmp} \n"
                         "QueryForLoop { --{iterator} {01} {07}\n"
                         "    select iterator as \"col1\", 'aiterator' as \"col2\"\n"
                         "}\n"
                         "}\n";


    DatabaseConnection dc;
    dc.disableSaveToUserDS = true;
    dc.Create("LOCAL","LOCAL");
    dc.execSql("Drop table __test_tmp");
    dc.execSql(selectStmt );
    dc.execSql("select * from __test_tmp");

    if( dc.data.tbldata[0][0] == "1" &&
        dc.data.tbldata[1][0] == "a01" &&
        dc.data.tbldata[0][1] == "2" &&
        dc.data.tbldata[1][1] == "a02" &&
        dc.data.tbldata[0][2] == "3" &&
        dc.data.tbldata[1][2] == "a03" &&
        dc.data.tbldata[0][3] == "4" &&
        dc.data.tbldata[1][3] == "a04" &&
        dc.data.tbldata[0][4] == "5" &&
        dc.data.tbldata[1][4] == "a05" &&
        dc.data.tbldata[0][5] == "6" &&
        dc.data.tbldata[1][5] == "a06" &&
        dc.data.tbldata[0][6] == "7" &&
        dc.data.tbldata[1][6] == "a07" )
        return 0;
    return 1;
}

int DateFunctionTest()
{
    QString selectStmt = "select DBLPasteMonth {{-1}}, "
                         "DBLPasteMonthDayOffset {{-1}}, "
                         "DBLPasteYearDayOffset {{-1}}, "
                         "DBLPasteYearMonthOffset {{-1}}, "
                         "DBLPasteDayOfMonth {{-1}}, "
                         "DBLPasteDaysinMonth {{-1}}, "
                         "DBLPasteDaysInMonthDayOffset {{-1}}, "
                         "DBLPasteDaysInMonthByMonth {{2}}\n";


    DatabaseConnection dc;
    dc.disableSaveToUserDS = true;
    dc.Create("LOCAL","LOCAL");
    dc.execSql(selectStmt );

    QDate dt = QDate::currentDate();
    QDate prevmonthdt = dt;
    prevmonthdt = prevmonthdt.addMonths(-1);
    QDate prevdaydt = dt;
    prevdaydt = prevdaydt.addDays(-1);

    QDate fixeddt = QDate(2025,2,1);


    if( dc.data.tbldata[0][0] ==  QVariant(prevmonthdt.month()).toString() &&
        dc.data.tbldata[1][0] ==  QVariant(prevdaydt.month()).toString() &&
        dc.data.tbldata[2][0] ==  QVariant(prevdaydt.year()).toString() &&
        dc.data.tbldata[3][0] ==  QVariant(prevmonthdt.year()).toString() &&
        dc.data.tbldata[4][0] ==  QVariant(prevdaydt.day()).toString() &&
        dc.data.tbldata[5][0] ==  QVariant(prevmonthdt.daysInMonth()).toString() &&
        dc.data.tbldata[6][0] ==  QVariant(prevmonthdt.daysInMonth()).toString() &&
        dc.data.tbldata[7][0] ==  QVariant(fixeddt.daysInMonth()).toString())
        return 0;
    qDebug() << dc.data.tbldata[0][0] <<  QVariant(prevmonthdt.month()).toString();
    qDebug() << dc.data.tbldata[1][0] <<  QVariant(prevdaydt.month()).toString();
    qDebug() << dc.data.tbldata[2][0] <<  QVariant(prevdaydt.year()).toString();
    qDebug() << dc.data.tbldata[3][0] <<  QVariant(prevmonthdt.year()).toString();
    qDebug() << dc.data.tbldata[4][0] <<  QVariant(prevdaydt.day()).toString();
    qDebug() << dc.data.tbldata[5][0] <<  QVariant(prevmonthdt.daysInMonth()).toString();
    qDebug() << dc.data.tbldata[6][0] <<  QVariant(prevmonthdt.daysInMonth()).toString();
    qDebug() << dc.data.tbldata[7][0] <<  QVariant(fixeddt.daysInMonth()).toString();
    return 1;
}


int ParallelExecTest()
{
    QString selectStmt = "StartAsyncExecution {{}}\n"
                         "ForLoop { --{iterator} {01} {07}\n"
                         "  SilentSubexecToLocalDBTable { {LOCAL} {LOCAL} {__test_tmp_iterator} \n"
                         "      select iterator as \"col1\", 'aiterator' as \"col2\"\n"
                         "  }\n"
                         "}\n"
                         "AwaitAsyncExecution {{}}\n"
                         "SilentSubexecToLocalDBTable { {LOCAL} {LOCAL} {__par_test_tmp} "
                         "  QueryForLoop { "
                         "  --{iterator} {01} {07}\n"
                         "    select * from __test_tmp_iterator\n"
                         "  }\n"
                         "}\n"
                         "select * from __par_test_tmp";


    DatabaseConnection dc;
    dc.disableSaveToUserDS = true;
    dc.Create("LOCAL","LOCAL");
    dc.execSql("Drop table __par_test_tmp");
    dc.execSql("Drop table __test_tmp_01");
    dc.execSql("Drop table __test_tmp_02");
    dc.execSql("Drop table __test_tmp_03");
    dc.execSql("Drop table __test_tmp_04");
    dc.execSql("Drop table __test_tmp_05");
    dc.execSql("Drop table __test_tmp_06");
    dc.execSql("Drop table __test_tmp_07");
    if(!dc.execSql(selectStmt))
        qDebug()<<"dc.execSql(selectStmt)) returned false";

    qDebug() << dc.data.tbldata[0][0] << dc.data.tbldata[1][0];
    qDebug() << dc.data.tbldata[0][1] << dc.data.tbldata[1][1];
    qDebug() << dc.data.tbldata[0][2] << dc.data.tbldata[1][2];
    qDebug() << dc.data.tbldata[0][3] << dc.data.tbldata[1][3];
    qDebug() << dc.data.tbldata[0][4] << dc.data.tbldata[1][4];
    qDebug() << dc.data.tbldata[0][5] << dc.data.tbldata[1][5];
    qDebug() << dc.data.tbldata[0][6] << dc.data.tbldata[1][6];


    if( dc.data.tbldata[0][0] == "1" &&
        dc.data.tbldata[1][0] == "a01" &&
        dc.data.tbldata[0][1] == "2" &&
        dc.data.tbldata[1][1] == "a02" &&
        dc.data.tbldata[0][2] == "3" &&
        dc.data.tbldata[1][2] == "a03" &&
        dc.data.tbldata[0][3] == "4" &&
        dc.data.tbldata[1][3] == "a04" &&
        dc.data.tbldata[0][4] == "5" &&
        dc.data.tbldata[1][4] == "a05" &&
        dc.data.tbldata[0][5] == "6" &&
        dc.data.tbldata[1][5] == "a06" &&
        dc.data.tbldata[0][6] == "7" &&
        dc.data.tbldata[1][6] == "a07" )
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
    if(test_case == "CycleTest")
        return CycleTest();
    if(test_case == "SubexecTest")
        return SubexecTest();
    if(test_case == "DateFunctionTest")
        return DateFunctionTest();
    if(test_case == "ParallelExecTest")
        return ParallelExecTest();
    if(test_case == "Amalgamation")// tun all tests in sequence a few times, in case this will somehow cause a crash
        return  xlsxReadWrite() + csvReadWrite() + LocalDBReadWrite() + CycleTest() + SubexecTest() + DateFunctionTest() + ParallelExecTest() +
                xlsxReadWrite() + csvReadWrite() + LocalDBReadWrite() + CycleTest() + SubexecTest() + DateFunctionTest() + ParallelExecTest();
    return 1;
}

#endif // TESTING_H
