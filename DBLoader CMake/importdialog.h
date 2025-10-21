#ifndef IMPORTDIALOG_H
#define IMPORTDIALOG_H

#include "tabledata.h"
#include <QDialog>

namespace Ui {
class ImportDialog;
}

class ImportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ImportDialog(QWidget *parent = nullptr);
    ~ImportDialog();

    void Setup();
    void Update();

    TableData* data = nullptr;

    int importType = 0;// 0 - excel, 1 - CSV, 2 - db


    bool csvIncludeHeaders = true;
    bool csvBypassToLocalDb = false;
    QString csvTableName = "";
    QString csvDelimeter = ";";
    QString csvLocalDbTableName = "tmp";


    bool excelIncludeHeaders = true;
    QString excelTableName = "";
    QString excelWorksheetName = "Sheet1";

    bool do_import = false;

private slots:
    void on_CSVBrowse_pushButton_clicked();

    void on_ExcelBrowse_pushButton_clicked();

    void on_import_pushButton_clicked();

    void on_pushButton_clicked();

private:
    Ui::ImportDialog *ui;
};

#endif // IMPORTDIALOG_H
