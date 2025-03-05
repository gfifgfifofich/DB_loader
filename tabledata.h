#ifndef TABLEDATA_H
#define TABLEDATA_H

#include <QObject>
#include <QQuickItem>

/*
TableData
    std::vector<std::vector<QVariant>> // slow
    std::vector<std::vector<QString>> // test, maybe would be faster
    std::vector<QString> headers
    std::vector<int> types // columns types

    void ImportFromCSV(QString fileName, QChar delimeter, bool firstRowHeader);
    void ImportFromExcel(QString fileName, int x_start,int x_end,int y_start,int y_end, bool firstRowHeader);
    void ImportFromSQLiteTable(QString fileName, QString tableName) // load table from sqlite quicker

    void ExportToCSV(QString fileName, QChar delimeter, bool firstRowHeader);
    void ExportToExcel(QString fileName, int x_start,int x_end,int y_start,int y_end, bool firstRowHeader);
    void ExportToSQLiteTable(QString fileName, QString tableName)

    Q_PROPERTY(int value READ value WRITE setValue NOTIFY valueChanged)
*/

class TableData : QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit TableData(QObject *parent = nullptr);

    void Init();

    QString sqlCode = "";
    QString allSqlCode = "";
    QString additionalSaveFileData = ""; // adds this in front of filename to ensure stuff. (except for SQLite tables)


    std::vector<std::vector<QVariant>> data; // slow
    QStringList headers;
    std::vector<int> types; // columns types

    Q_INVOKABLE void ImportFromCSV(QString fileName, QChar delimeter, bool firstRowHeader);
    Q_INVOKABLE void ImportFromExcel(QString fileName, int x_start,int x_end,int y_start,int y_end, bool firstRowHeader);
    Q_INVOKABLE void ImportFromSQLiteTable(QString fileName, QString tableName); // load table from sqlite quicker


    Q_INVOKABLE bool ExportToCSV(QString fileName, char delimeter, bool firstRowHeader);
    Q_INVOKABLE bool ExportToExcel(QString fileName, int x_start,int x_end,int y_start,int y_end, bool firstRowHeader);
    Q_INVOKABLE bool ExportToSQLiteTable(QString tableName);


signals:
    void ImportedFromCSV();
    void ImportedFromExcel();
    void ImportedFromSQLiteTable();

    void ExportedToCSV();
    void ExportedToExcel();
    void ExportedToSQLiteTable();

};

#endif // TABLEDATA_H
