#ifndef EXPORTDIALOG_H
#define EXPORTDIALOG_H

#include <QDialog>
#include "tabledata.h"
namespace Ui {
class ExportDialog;
}

class ExportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ExportDialog(QWidget *parent = nullptr);
    ~ExportDialog();

    void Setup();
    void Update();

    TableData* data = nullptr;

    int exportType = 0;// 0 - excel, 1 - CSV, 2 - db

    bool dbAppend = false;
    QString dbTablename = "tmp";
    QString dbConnectionName = "";

    bool csvAppend = false;
    bool csvIncludeHeaders = true;
    QString csvTableName = "";
    QString csvDelimeter = ";";

    bool excelSyncUniqueColumn = false;
    bool excelReplaceSheet = false;
    bool excelAppendSheet = false;
    bool excelIncludeHeaders = true;
    QString excelTableName = "";
    QString excelWorksheetName = "Sheet1";
    QString excelUniqueColumnName = "";

    bool do_export = false;


private slots:
    void on_Export_pushButton_clicked();

    void on_Cancel_pushButton_clicked();

    void on_ExcelBrowse_pushButton_clicked();

    void on_CSVBrowse_pushButton_clicked();

private:
    Ui::ExportDialog *ui;
};

#endif // EXPORTDIALOG_H
