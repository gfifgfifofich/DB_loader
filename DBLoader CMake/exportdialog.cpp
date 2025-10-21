#include "exportdialog.h"
#include "datastorage.h"
#include "ui_exportdialog.h"
#include <QFileDialog>

inline QString usrDir;
inline QString documentsDir;
inline DataStorage userDS;
inline DataStorage userExportDS;

ExportDialog::ExportDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ExportDialog)
{

    ui->setupUi(this);


    connect(ui->CSVAppend_radioButton,&QRadioButton::clicked,this,&ExportDialog::Update);
    connect(ui->CSVReplace_radioButton,&QRadioButton::clicked,this,&ExportDialog::Update);
    connect(ui->DBAppend_radioButton,&QRadioButton::clicked,this,&ExportDialog::Update);
    connect(ui->DBReplace_radioButton,&QRadioButton::clicked,this,&ExportDialog::Update);
    connect(ui->ExcelAppendSheet_radioButton,&QRadioButton::clicked,this,&ExportDialog::Update);
    connect(ui->ExcelReplaceSheet_radioButton,&QRadioButton::clicked,this,&ExportDialog::Update);
    connect(ui->ExcelReplace_radioButton,&QRadioButton::clicked,this,&ExportDialog::Update);
    connect(ui->CSVDelimeterComma_radioButton,&QRadioButton::clicked,this,&ExportDialog::Update);
    connect(ui->CSVDelimeterSemicolumn_radioButton,&QRadioButton::clicked,this,&ExportDialog::Update);
    connect(ui->CSVDelimeterSpace_radioButton,&QRadioButton::clicked,this,&ExportDialog::Update);
    connect(ui->CSVDelimeterTab_radioButton,&QRadioButton::clicked,this,&ExportDialog::Update);

    connect(ui->ExcelAppendSync_checkBox,&QPushButton::clicked,this,&ExportDialog::Update);
    connect(ui->ExcelHeaders_checkBox,&QPushButton::clicked,this,&ExportDialog::Update);
    connect(ui->CSVHeaders_checkBox,&QPushButton::clicked,this,&ExportDialog::Update);
}

ExportDialog::~ExportDialog()
{
    delete ui;
}

void ExportDialog::Setup()
{
    if(data!=nullptr)
    {
        ui->ExcelSyncColumn_comboBox->clear();
        ui->ExcelSyncColumn_comboBox->addItems(data->headers);

        ui->DBConnection_comboBox->clear();
        QStringList strl;
        for(auto x : userDS.data["Connections"])
            strl.push_back(x.first);
        ui->DBConnection_comboBox->addItems(strl);

    }

    userExportDS.Load(documentsDir + "/lastExportSettings.txt");

    if(userExportDS.data["export"].size() <=0)
    {// fill with standart data

        QString csv_str = documentsDir + "/CSV/csvExport.csv";
        QString excel_str = documentsDir + "/excel/excelExport.xlsx";


        userExportDS.SetProperty("export","exportType", 0);

        userExportDS.SetProperty("export","dbAppend", false);
        userExportDS.data["export"]["dbTablename"] =  "tmp";

        userExportDS.SetProperty("export","dbConnectionName", "");

        userExportDS.SetProperty("export","csvAppend", false);
        userExportDS.SetProperty("export","csvIncludeHeaders", true);
        userExportDS.SetProperty("export","csvTableName", csv_str);
        userExportDS.data["export"]["csvDelimeter"] =  ";";


        userExportDS.SetProperty("export","excelSyncUniqueColumn", false);
        userExportDS.SetProperty("export","excelReplaceSheet", false);
        userExportDS.SetProperty("export","excelAppendSheet", false);
        userExportDS.SetProperty("export","excelIncludeHeaders", true);
        userExportDS.SetProperty("export","excelTableName", excel_str);
        userExportDS.data["export"]["excelWorksheetName"] = "Sheet1";

    }
    if(userExportDS.GetPropertyAsBool("export","csvAppend"))
    {

        ui->CSVAppend_radioButton->setChecked(true);
        ui->CSVReplace_radioButton->setChecked(false);
    }

    if(userExportDS.GetPropertyAsBool("export","excelAppendSheet"))
    {
        ui->ExcelAppendSheet_radioButton->setChecked(true);
        ui->ExcelReplaceSheet_radioButton->setChecked(false);
        ui->ExcelReplace_radioButton->setChecked(false);
    }

    else if(userExportDS.GetPropertyAsBool("export","excelReplaceSheet"))
    {
        ui->ExcelAppendSheet_radioButton->setChecked(false);
        ui->ExcelReplaceSheet_radioButton->setChecked(true);
        ui->ExcelReplace_radioButton->setChecked(false);
    }
    else
    {
        ui->ExcelAppendSheet_radioButton->setChecked(false);
        ui->ExcelReplaceSheet_radioButton->setChecked(false);
        ui->ExcelReplace_radioButton->setChecked(true);
    }
    if(userExportDS.GetPropertyAsBool("export","excelSyncUniqueColumn"))
        ui->ExcelAppendSync_checkBox->setChecked(true);
    else
        ui->ExcelAppendSync_checkBox->setChecked(false);

    ui->ExcelHeaders_checkBox->setChecked(userExportDS.GetPropertyAsBool("export","excelIncludeHeaders"));
    ui->CSVHeaders_checkBox->setChecked(userExportDS.GetPropertyAsBool("export","csvIncludeHeaders"));

    ui->DBAppend_radioButton->setChecked(userExportDS.GetPropertyAsBool("export","dbAppend"));
    ui->DBReplace_radioButton->setChecked(!userExportDS.GetPropertyAsBool("export","dbAppend"));
    ui->DBConnection_comboBox->setCurrentText(userExportDS.GetProperty("export","dbConnectionName").trimmed());

    ui->DB_TableName->setText(userExportDS.GetProperty("export","dbTablename").trimmed());
    ui->CSV_TableName->setText(userExportDS.GetProperty("export","csvTableName").trimmed());
    ui->Excel_TableName->setText(userExportDS.GetProperty("export","excelTableName").trimmed());

    ui->ExcelWorksheetName_lineEdit->setText(userExportDS.GetProperty("export","excelWorksheetName").trimmed());

    ui->CSVDelimeterSemicolumn_radioButton->setChecked(userExportDS.GetProperty("export","csvDelimeter").trimmed().contains(';'));
    ui->CSVDelimeterComma_radioButton->setChecked(userExportDS.GetProperty("export","csvDelimeter").trimmed().contains(','));
    ui->CSVDelimeterSpace_radioButton->setChecked(userExportDS.GetProperty("export","csvDelimeter").trimmed().contains("space"));
    ui->CSVDelimeterTab_radioButton->setChecked(userExportDS.GetProperty("export","csvDelimeter").trimmed().contains("tab"));


    exportType = userExportDS.GetPropertyAsInt("export","exportType");


    if(exportType == 0)
        ui->tabWidget->setCurrentIndex(0);
    else if(exportType == 1)
        ui->tabWidget->setCurrentIndex(1);
    else if(exportType == 2)
        ui->tabWidget->setCurrentIndex(2);
    else
        ui->tabWidget->setCurrentIndex(0);


    Update();
}

void ExportDialog::Update()
{

    exportType=ui->tabWidget->currentIndex();
    if(exportType<0 || exportType > 2)
        exportType = 0;

    dbAppend = ui->DBAppend_radioButton->isChecked();
    dbTablename = ui->DB_TableName->text().trimmed();
    dbConnectionName = ui->DBConnection_comboBox->currentText().trimmed();

    csvAppend = ui->CSVAppend_radioButton->isChecked();
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

    excelAppendSheet = ui->ExcelAppendSheet_radioButton->isChecked();
    excelReplaceSheet = ui->ExcelReplaceSheet_radioButton->isChecked();
    excelIncludeHeaders = ui->ExcelHeaders_checkBox->isChecked();
    excelTableName = ui->Excel_TableName->text().trimmed();
    excelWorksheetName = ui->ExcelWorksheetName_lineEdit->text().trimmed();
    excelSyncUniqueColumn = ui->ExcelAppendSync_checkBox->isChecked();
    excelUniqueColumnName = ui->ExcelSyncColumn_comboBox->currentText();

    userExportDS.SetProperty("export","exportType", exportType);

    userExportDS.SetProperty("export","dbAppend", dbAppend);
    userExportDS.data["export"]["dbTablename"] =  dbTablename;

    userExportDS.SetProperty("export","dbConnectionName", dbConnectionName);

    userExportDS.SetProperty("export","csvAppend", csvAppend);
    userExportDS.SetProperty("export","csvIncludeHeaders", csvIncludeHeaders);
    userExportDS.SetProperty("export","csvTableName", csvTableName);
    userExportDS.data["export"]["csvDelimeter"] =  csvDelimeter;


    userExportDS.SetProperty("export","excelSyncUniqueColumn", excelSyncUniqueColumn);
    userExportDS.SetProperty("export","excelAppendSheet", excelAppendSheet);
    userExportDS.SetProperty("export","excelReplaceSheet", excelReplaceSheet);
    userExportDS.SetProperty("export","excelIncludeHeaders", excelIncludeHeaders);
    userExportDS.SetProperty("export","excelTableName", excelTableName);
    userExportDS.data["export"]["excelWorksheetName"] = excelWorksheetName;


    if(excelAppendSheet || excelReplaceSheet)
    {
        ui->ExcelWorksheetName_lineEdit->show();
        if(excelAppendSheet)
        {
            ui->ExcelAppendSync_checkBox->show();
            if(excelSyncUniqueColumn)
                ui->ExcelSyncColumn_comboBox->show();
            else
                ui->ExcelSyncColumn_comboBox->hide();
        }
        else
            ui->ExcelAppendSync_checkBox->hide();
    }
    else
    {
        ui->ExcelWorksheetName_lineEdit->hide();
        ui->ExcelAppendSync_checkBox->hide();
        ui->ExcelSyncColumn_comboBox->hide();
    }




}

void ExportDialog::on_Export_pushButton_clicked()
{
    Update();
    do_export = true;
    userExportDS.Save(documentsDir + "/lastExportSettings.txt");
    close();
}


void ExportDialog::on_Cancel_pushButton_clicked()
{
    do_export = false;
    close();
}


void ExportDialog::on_ExcelBrowse_pushButton_clicked()
{
    QString file = QFileDialog::getOpenFileName(this, tr("Select csv file"),ui->Excel_TableName->text());

    if(file.size()>0)
        ui->Excel_TableName->setText(file);
}


void ExportDialog::on_CSVBrowse_pushButton_clicked()
{
    QString file = QFileDialog::getOpenFileName(this, tr("Select csv file"),ui->CSV_TableName->text());

    if(file.size()>0)
        ui->CSV_TableName->setText(file);
}

