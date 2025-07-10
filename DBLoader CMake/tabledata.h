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
    QString LastUser = "";
    QString LastDriver = "";
    QString LastDatabase = "";
    QString sqlCode = "";
    QString allSqlCode = "";
    QString additionalSaveFileData = "";


    QTime LastSaveEndDate;
    QTime LastSaveDuration;
    quint64 saveRowSize = 0;
    quint64 saveRowsDone = 0;
    bool lastExportSuccess = false;
    bool exporting = false;
    bool exported = false;
    QString lastexporttype = "";
    bool stopNow = false;


    //Table data
    QVector<QVector<QString>> tbldata;
    QStringList headers;
    std::vector<int> types; // columns types
    std::vector<std::vector<int>> typecount; // column has a string in it. Force string on whole column
    std::vector<int> maxVarTypes; // columns types

    // import
    Q_INVOKABLE bool ImportFromCSV(QString fileName, QChar delimeter, bool firstRowHeader);
    Q_INVOKABLE bool ImportFromExcel(QString fileName, int x_start,int x_end,int y_start,int y_end, bool firstRowHeader);
    Q_INVOKABLE void ImportFromSQLiteTable(QString fileName, QString tableName); // load table from sqlite quicker

    // export
    Q_INVOKABLE bool ExportToCSV(QString fileName, char delimeter, bool firstRowHeader);
    Q_INVOKABLE bool ExportToExcel(QString fileName, int x_start,int x_end,int y_start,int y_end, bool firstRowHeader,QString sheetName = "", bool append = false);
    Q_INVOKABLE bool ExportToSQLiteTable(QString tableName);

    Q_INVOKABLE bool AppendToCSV(QString fileName, char delimeter);
    Q_INVOKABLE bool AppendToExcel(QString fileName, QString SheetName = "Sheet1");

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



signals:
    void ImportedFromCSV();
    void ImportedFromExcel();
    void ImportedFromSQLiteTable();

    void ExportedToCSV();
    void ExportedToExcel();
    void ExportedToSQLiteTable();

};

#endif // TABLEDATA_H
