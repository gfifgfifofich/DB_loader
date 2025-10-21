#include "importdialog.h"
#include "datastorage.h"
#include "ui_importdialog.h"
#include <QFileDialog>

inline QString usrDir;
inline QString documentsDir;
inline DataStorage userDS;
inline DataStorage userimportDS;

ImportDialog::ImportDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ImportDialog)
{
    ui->setupUi(this);
}

ImportDialog::~ImportDialog()
{
    delete ui;
}

void ImportDialog::Setup()
{

    userimportDS.Load(documentsDir + "/lastExportSettings.txt");

    if(userimportDS.data["import"].size() <=0)
    {// fill with standart data

        QString csv_str = documentsDir + "/CSV/csvimport.csv";
        QString excel_str = documentsDir + "/excel/excelimport.xlsx";


        userimportDS.SetProperty("import","importType", 0);

        userimportDS.SetProperty("import","csvBypassToLocalDb", false);
        userimportDS.SetProperty("import","csvIncludeHeaders", true);
        userimportDS.SetProperty("import","csvTableName", csv_str);
        userimportDS.data["import"]["csvDelimeter"] =  ";";
        userimportDS.data["import"]["csvLocalDbTableName"] =  "tmp";


        userimportDS.SetProperty("import","excelIncludeHeaders", true);
        userimportDS.SetProperty("import","excelTableName", excel_str);

    }



    ui->ExcelHeaders_checkBox->setChecked(userimportDS.GetPropertyAsBool("import","excelIncludeHeaders"));
    ui->CSVHeaders_checkBox->setChecked(userimportDS.GetPropertyAsBool("import","csvIncludeHeaders"));
    ui->CSVBypass_checkBox->setChecked(userimportDS.GetPropertyAsBool("import","csvBypassToLocalDb"));
    ui->CSVBypassTablename_lineEdit->setText(userimportDS.GetProperty("import","csvLocalDbTableName").trimmed());

    ui->CSV_TableName->setText(userimportDS.GetProperty("import","csvTableName").trimmed());
    ui->Excel_TableName->setText(userimportDS.GetProperty("import","excelTableName").trimmed());

    ui->ExcelWorksheetName_lineEdit->setText(userimportDS.GetProperty("import","excelWorksheetName").trimmed());

    ui->CSVDelimeterSemicolumn_radioButton->setChecked(userimportDS.GetProperty("import","csvDelimeter").trimmed().contains(';'));
    ui->CSVDelimeterComma_radioButton->setChecked(userimportDS.GetProperty("import","csvDelimeter").trimmed().contains(','));
    ui->CSVDelimeterSpace_radioButton->setChecked(userimportDS.GetProperty("import","csvDelimeter").trimmed().contains("space"));
    ui->CSVDelimeterTab_radioButton->setChecked(userimportDS.GetProperty("import","csvDelimeter").trimmed().contains("tab"));


    importType = userimportDS.GetPropertyAsInt("import","importType");


    if(importType == 0)
        ui->tabWidget->setCurrentIndex(0);
    else if(importType == 1)
        ui->tabWidget->setCurrentIndex(1);
    else
        ui->tabWidget->setCurrentIndex(0);


    Update();
}

void ImportDialog::Update()
{

    importType =ui->tabWidget->currentIndex();
    if(importType <0 || importType  > 1)
        importType  = 0;


    csvIncludeHeaders = ui->CSVHeaders_checkBox->isChecked();
    csvTableName = ui->CSV_TableName->text().trimmed();

    if(ui->CSVDelimeterSemicolumn_radioButton->isChecked())
        csvDelimeter = ";";
    else if(ui->CSVDelimeterComma_radioButton->isChecked())
        csvDelimeter = ",";
    else if(ui->CSVDelimeterSpace_radioButton->isChecked())
        csvDelimeter = "space";
    else if(ui->CSVDelimeterTab_radioButton->isChecked())
        csvDelimeter = "tab";

    excelIncludeHeaders = ui->ExcelHeaders_checkBox->isChecked();
    excelTableName = ui->Excel_TableName->text().trimmed();
    excelWorksheetName = ui->ExcelWorksheetName_lineEdit->text().trimmed();

    csvBypassToLocalDb = ui->CSVBypass_checkBox->isChecked();
    csvLocalDbTableName = ui->CSVBypassTablename_lineEdit->text().trimmed();

    userimportDS.SetProperty("import","importType", importType);

    userimportDS.SetProperty("import","csvIncludeHeaders", csvIncludeHeaders);
    userimportDS.SetProperty("import","csvTableName", csvTableName);
    userimportDS.data["import"]["csvDelimeter"] =  csvDelimeter;
    userimportDS.SetProperty("import","csvBypassToLocalDb", csvBypassToLocalDb );
    userimportDS.data["import"]["csvLocalDbTableName"] = csvLocalDbTableName;

    userimportDS.SetProperty("import","excelIncludeHeaders", excelIncludeHeaders);
    userimportDS.SetProperty("import","excelTableName", excelTableName);
    userimportDS.data["import"]["excelWorksheetName"] = excelWorksheetName;





}

void ImportDialog::on_CSVBrowse_pushButton_clicked()
{
    QString file = QFileDialog::getOpenFileName(this, tr("Select csv file"),ui->CSV_TableName->text());

    if(file.size()>0)
        ui->CSV_TableName->setText(file);
}


void ImportDialog::on_ExcelBrowse_pushButton_clicked()
{

    QString file = QFileDialog::getOpenFileName(this, tr("Select csv file"),ui->Excel_TableName->text());

    if(file.size()>0)
        ui->Excel_TableName->setText(file);
}



void ImportDialog::on_import_pushButton_clicked()
{

    Update();
    do_import = true;
    userimportDS.Save(documentsDir + "/lastExportSettings.txt");
    close();
}


void ImportDialog::on_pushButton_clicked()
{

    close();
}

