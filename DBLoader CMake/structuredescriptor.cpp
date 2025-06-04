#include "structuredescriptor.h"
#include "ui_structuredescriptor.h"
#include "loaderwidnow.h"
#include <qshortcut.h>


inline QString documentsDir;
StructureDescriptor::StructureDescriptor(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::StructureDescriptor)
{
    ui->setupUi(this);

    cd = new CodeEditor();


    new QShortcut(QKeySequence(Qt::Key_Escape), this, [this]() {
        close();
    });
    new QShortcut(QKeySequence(Qt::Key_Control | Qt::Key_D), this, [this]() {
        close();
    });


    ui->verticalLayout_3->addWidget(cd);

    cd->b_codePreview = false;

    cd->setPlainText(
        "SELECT distinct lower(table_name), lower(column_name )  -- oracle\n"
        "FROM ALL_TAB_COLUMNS       \n"
        "inner join   (SELECT DISTINCT  OBJECT_NAME       \n"
        "    FROM ALL_OBJECTS           \n"
        "    WHERE OBJECT_TYPE = 'TABLE' or OBJECT_TYPE like '%VIEW%'  AND\n"
        "    (upper(OWNER) != 'SYSTEM' AND upper(OWNER) != 'SYS' AND upper(OWNER) != 'ADMIN' )\n"
        ") tables on tables.OBJECT_NAME = table_name      \n"
        "order by lower(table_name)"
        "\n"
        "\n"
        "\n"
        "--SELECT table_name, column_name -- Postgres\n"
        "--  FROM information_schema.columns\n"
        "-- WHERE table_schema = 'public';    \n"
            "\n"
            "\n"
            "\n"
        "--SQLite is automatic, just press connect again\n"
        );


    for(auto table : loadWind->cd->highlighter->TableColumnDS.data)
    {
        ui->listWidget->addItem(table.first);

    }
}


StructureDescriptor::~StructureDescriptor()
{
    delete ui;
}

void StructureDescriptor::Init(QString selecteWord)
{
    ui->lineEdit->setText(selecteWord);
    ui->lineEdit_2->setText(selecteWord);
}

void StructureDescriptor::on_pushButton_clicked()
{// Update button
    if(loadWind->cd != nullptr)
    {


        loadWind->dc->execSql(cd->toPlainText());
        loadWind->UpdateTable();
        if(loadWind->dc->data.tbldata.size()>=2 && loadWind->dc->data.tbldata[0].size()>0)
        {

            loadWind->cd->highlighter->TableColumnDS.data.clear();
            for(int i = 0; i < loadWind->dc->data.tbldata[0].size();i++)
            {
                loadWind->cd->highlighter->TableColumnDS.data[loadWind->dc->data.tbldata[0][i].toString()][loadWind->dc->data.tbldata[1][i].toString()];
            }


            QString filename = documentsDir + "/"+ loadWind->cd->highlighter->dbSchemaName + ".txt";

            loadWind->cd->highlighter->TableColumnDS.Save(filename);
        }
    }
}


void StructureDescriptor::on_lineEdit_2_textChanged(const QString &arg1)
{// Tables filter changed
    tableStr = arg1;
    updateTableSearch();
}

void StructureDescriptor::on_lineEdit_3_textChanged(const QString &arg1)
{// Tables filter by column changed
    columnStr = arg1;
    updateTableSearch();
}


void StructureDescriptor::updateTableSearch()
{
    if(loadWind->cd != nullptr)
    {
        ui->listWidget->clear();

        for(auto table : loadWind->cd->highlighter->TableColumnDS.data)
        {
            if(tableStr.trimmed().size() > 0 || columnStr.trimmed().size() > 0)
            {
                bool hasColumn = true;
                if(columnStr.trimmed().size() > 0)
                {
                    hasColumn = false;
                    for(auto column : loadWind->cd->highlighter->TableColumnDS.data[QString(table.first).trimmed()])
                    {
                        if(QString(column.first).trimmed().contains(columnStr))
                        {
                            hasColumn = true;
                            break;
                        }
                    }
                }

                if(QString(table.first).trimmed().startsWith(tableStr) && hasColumn)
                    ui->listWidget->addItem(table.first);
            }
            else
                ui->listWidget->addItem(table.first);

        }
        for(auto table : loadWind->cd->highlighter->TableColumnDS.data)
        {
            if(tableStr.trimmed().size() > 0 || columnStr.trimmed().size() > 0)
            {
                bool hasColumn = true;
                if(columnStr.trimmed().size() > 0)
                {
                    hasColumn = false;
                    for(auto column : loadWind->cd->highlighter->TableColumnDS.data[QString(table.first).trimmed()])
                    {
                        if(QString(column.first).trimmed().contains(columnStr))
                        {
                            hasColumn = true;
                            break;
                        }
                    }
                }

                if(QString(table.first).trimmed().contains(tableStr) && hasColumn && !QString(table.first).trimmed().startsWith(tableStr))
                    ui->listWidget->addItem(table.first);
            }
            else
                ui->listWidget->addItem(table.first);

        }

    }

}


void StructureDescriptor::on_lineEdit_textChanged(const QString &arg1)
{// Describing table name changed
    if(loadWind->cd != nullptr)
    {
        ui->listWidget_2->clear();
        for(auto column : loadWind->cd->highlighter->TableColumnDS.data[arg1.trimmed()])
        {
            ui->listWidget_2->addItem(column.first);
        }
    }
}

void StructureDescriptor::on_listWidget_currentTextChanged(const QString &currentText)
{// Database tables changed
    ui->lineEdit->setText(currentText);
}


void StructureDescriptor::on_listWidget_2_currentTextChanged(const QString &currentText)
{// Table column changed
    if(loadWind->cd != nullptr)
    {

    }
}



void StructureDescriptor::on_listWidget_2_itemDoubleClicked(QListWidgetItem *item)
{
    QString str = ui->lineEdit->text();

    ui->lineEdit_3->setText(item->text());
    ui->lineEdit_2->setText("");

    ui->lineEdit->setText(str);
}

