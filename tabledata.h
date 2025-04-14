#ifndef TABLEDATA_H
#define TABLEDATA_H

#include <QObject>
#include <QQuickItem>
#include <qabstractitemmodel.h>
#include <qsqldatabase.h>
#include <qsqlquery.h>
#include <QMutex>

#include "QXlsx-master/QXlsx/header/xlsxdocument.h"
#include "QXlsx-master/QXlsx/header/xlsxworkbook.h"


class TableData : public QAbstractTableModel
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit TableData(QObject *parent = nullptr);

    void Init();

    // excel 'Debug' info
    QString sqlCode = "";
    QString allSqlCode = "";
    QString additionalSaveFileData = "";

    //Table data
    QVector<QVector<QVariant>> tbldata;
    QStringList headers;
    std::vector<int> types; // columns types

    // import
    Q_INVOKABLE void ImportFromCSV(QString fileName, QChar delimeter, bool firstRowHeader);
    Q_INVOKABLE void ImportFromExcel(QString fileName, int x_start,int x_end,int y_start,int y_end, bool firstRowHeader);
    Q_INVOKABLE void ImportFromSQLiteTable(QString fileName, QString tableName); // load table from sqlite quicker

    // export
    Q_INVOKABLE bool ExportToCSV(QString fileName, char delimeter, bool firstRowHeader);
    Q_INVOKABLE bool ExportToExcel(QString fileName, int x_start,int x_end,int y_start,int y_end, bool firstRowHeader);
    Q_INVOKABLE bool ExportToSQLiteTable(QString tableName);

    // QAbstractTableModel overrides
    int rowCount(const QModelIndex & = QModelIndex()) const override;
    int columnCount(const QModelIndex & = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;



    //QML pass through of QXlsx functions
    // used in point redact stuff, mostly just to pass QXlsx functionality to qml
    QXlsx::Document* xlsxFile = nullptr;

    Q_INVOKABLE QVariant getObject(int x,int y);// qml functionality
    Q_INVOKABLE int getSizeX();// qml functionality
    Q_INVOKABLE int getSizeY();// qml functionality
    Q_INVOKABLE void selectXLSXFile(QString filename);// qml functionality
    Q_INVOKABLE bool saveXLSXFile(QString filename);// qml functionality
    Q_INVOKABLE void writeToXLSXFile(int x, int y, QVariant value);// qml functionality
    Q_INVOKABLE QVariant readFromXLSXFile(int x, int y);// qml functionality

    // move table(x_s,y_s,x_s,y_s) by dx, dy
    // ExcelMove(0,0,10,10, 5,5); // move block 5 right 5 down
    Q_INVOKABLE bool ExcelMove(QString table,int x_start,int x_end,int y_start,int y_end, int dx, int dy);


signals:
    void ImportedFromCSV();
    void ImportedFromExcel();
    void ImportedFromSQLiteTable();

    void ExportedToCSV();
    void ExportedToExcel();
    void ExportedToSQLiteTable();

};

#endif // TABLEDATA_H
