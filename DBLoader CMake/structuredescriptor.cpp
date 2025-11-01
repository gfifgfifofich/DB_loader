#include "structuredescriptor.h"
#include "ui_structuredescriptor.h"
#include "loaderwidnow.h"
#include <qshortcut.h>


inline QString documentsDir;
inline DataStorage userDS;
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

        if(loadWind->dc->oracle)
        {
            loadWind->dc->execSql("SELECT distinct lower(table_name), lower(column_name )  -- oracle\n"
                                  "FROM ALL_TAB_COLUMNS       \n"
                                  "inner join   (SELECT DISTINCT  OBJECT_NAME       \n"
                                  "    FROM ALL_OBJECTS           \n"
                                  "    WHERE OBJECT_TYPE = 'TABLE' or OBJECT_TYPE like '%VIEW%'  AND\n"
                                  "    (upper(OWNER) != 'SYSTEM' AND upper(OWNER) != 'SYS' AND upper(OWNER) != 'ADMIN' )\n"
                                  ") tables on tables.OBJECT_NAME = table_name      \n"
                                  "order by lower(table_name)");
        }
        if(loadWind->dc->postgre)
        {
            loadWind->dc->execSql("SELECT table_name, column_name -- Postgres\n"
                                  "  FROM information_schema.columns\n"
                                  " WHERE table_schema = 'public'\n");
        }

        loadWind->UpdateTable();
        if(loadWind->dc->data.tbldata.size()>=2 && loadWind->dc->data.tbldata[0].size()>0)
        {

            loadWind->cd->highlighter->TableColumnDS.data.clear();
            for(int i = 0; i < loadWind->dc->data.tbldata[0].size();i++)
            {
                loadWind->cd->highlighter->TableColumnDS.data[loadWind->dc->data.tbldata[0][i]][loadWind->dc->data.tbldata[1][i]];
            }


            QString filename = documentsDir + "/"+ loadWind->cd->highlighter->dbSchemaName + ".txt";

            loadWind->cd->highlighter->TableColumnDS.Save(filename);
        }


        if(loadWind->dc->oracle)
        {
            loadWind->dc->execSql("SELECT distinct lower(table_name), lower(column_name ), lower(owner)  -- oracle\n"
                                  "FROM DBA_TAB_COLUMNS       \n"
                                  "inner join   (SELECT DISTINCT  OBJECT_NAME       \n"
                                  "    FROM DBA_OBJECTS           \n"
                                  "    WHERE OBJECT_TYPE = 'TABLE' or OBJECT_TYPE like '%VIEW%'  AND\n"
                                  "    (upper(OWNER) != 'SYSTEM' AND upper(OWNER) != 'SYS' AND upper(OWNER) != 'ADMIN' )\n"
                                  ") tables on tables.OBJECT_NAME = table_name      \n"
                                  "order by lower(table_name)");
        }
        if(loadWind->dc->postgre)
        {
            loadWind->dc->execSql("SELECT table_name, column_name, table_schema -- Postgres\n"
                                  "  FROM information_schema.columns\n");
        }

        loadWind->UpdateTable();
        if(loadWind->dc->data.tbldata.size()>=3 && loadWind->dc->data.tbldata[0].size()>0)
        {

            loadWind->cd->highlighter->AllTableColumnDS.data.clear();
            for(int i = 0; i < loadWind->dc->data.tbldata[0].size();i++)
            {
                loadWind->cd->highlighter->AllTableColumnDS.data[loadWind->dc->data.tbldata[0][i]][loadWind->dc->data.tbldata[1][i]];
                loadWind->cd->highlighter->AllTableColumnDS.data[loadWind->dc->data.tbldata[0][i]]["this_table_schema_name"] = loadWind->dc->data.tbldata[2][i];
            }


            QString filename = documentsDir + "/All_"+ loadWind->cd->highlighter->dbSchemaName + ".txt";

            loadWind->cd->highlighter->AllTableColumnDS.Save(filename);
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

void StructureDescriptor::on_Schema_lineEdit_textChanged(const QString &arg1)
{// schema name filter
    schemaStr = arg1;
    if(schemaStr.trimmed().size()<=0)
        ignoreSchema = true;
    else
        ignoreSchema = false;
    updateTableSearch();

}


void StructureDescriptor::updateTableSearch()
{
    if(loadWind->cd != nullptr)
    {
        ui->listWidget->clear();

        DataStorage* ds = &loadWind->cd->highlighter->AllTableColumnDS;
        if(ds->data.size() <=0)
            ds = &loadWind->cd->highlighter->TableColumnDS;

        for(auto table : ds->data)
        {
            QString schema = ds->data[table.first]["this_table_schema_name"].trimmed() + ".";
            if(schema.trimmed()=="." || ignoreSchema)
                schema = "";
            else
                if(!schema.contains(schemaStr))
                    continue;
            if(tableStr.trimmed().size() > 0 || columnStr.trimmed().size() > 0)
            {
                bool hasColumn = true;
                if(columnStr.trimmed().size() > 0)
                {
                    hasColumn = false;
                    for(auto column : ds->data[QString(table.first).trimmed()])
                    {
                        if(QString(column.first).trimmed().contains(columnStr))
                        {
                            hasColumn = true;
                            break;
                        }
                    }
                }

                if(QString(table.first).trimmed().startsWith(tableStr) && hasColumn)
                    ui->listWidget->addItem(schema + table.first);



            }
            else
                ui->listWidget->addItem(schema + table.first);

        }

    }

}


void StructureDescriptor::on_lineEdit_textChanged(const QString &arg1)
{// Describing table name changed
    DataStorage* ds = &loadWind->cd->highlighter->AllTableColumnDS;
    if(ds->data.size() <=0)
        ds = &loadWind->cd->highlighter->TableColumnDS;

    if(loadWind->cd != nullptr)
    {
        QString asd = arg1;
        if(!ignoreSchema || schemaStr.size() >0)
            while(asd.size() > 0)
            {
                if(asd[0]!='.')
                    asd.removeAt(0);
                else
                {
                    asd.removeAt(0);
                    break;

                }
            }

        qDebug() << "loadWind->dc->conname.trimmed()" << loadWind->dc->conname.trimmed() << userDS.data["UserTheme"]["db_drv_Save_table_Connection"].trimmed();
        if(loadWind->dc->conname.trimmed() == userDS.data["UserTheme"]["db_drv_Save_table_Connection"].trimmed())
        {// in local db
            qDebug() << "in local database";
            DatabaseConnection dc;
            // dc.nodebug = true;
            dc.rawquery = true;
            dc.disableSaveToUserDS = true;
            dc.Create(userDS.data["UserTheme"]["db_drv_Save_table_Connection"].trimmed());




            if(dc.execSql("select exec_sql,cre_date,exec_workspace_name,exec_user_name,exec_driver,exec_database from user_table_desc where exec_table_name = '" + asd.trimmed() +  "'"))
            {
                if(dc.data.tbldata.size() >=6 && dc.data.tbldata[0].size()>0)
                {
                    cd->setPlainText("-- {" + dc.data.tbldata[4][0] + "} {" + dc.data.tbldata[5][0] + "} \n"
                                     "--Last update: " + dc.data.tbldata[1][0] + "\n" +
                                     "--Last workspace: " + dc.data.tbldata[2][0] + "\n" +
                                     "--Last user: " + dc.data.tbldata[3][0] + "\n" +
                                     dc.data.tbldata[0][0]);
                }
            }
        }

        ui->listWidget_2->clear();
        for(auto y : ds->data[asd.trimmed()])
        {
            if(y.first.contains("this_table_schema_name"))
                continue;
            QString word = y.first.trimmed() +' '+ y.second;
            if(y.second.trimmed().size()<=0)
                word = y.first;
            word = word.trimmed();
            ui->listWidget_2->addItem((word));
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
    if(ui->recordChainCheckBox->isChecked())
    {
        chain.push_back({chainJoinTable,ui->lineEdit->text(),item->text()});
        Qt::KeyboardModifiers modifiers  = QApplication::queryKeyboardModifiers ();
        if(!modifiers.testFlag(Qt::KeyboardModifier::ControlModifier))
        {
            chainJoinTable = ui->lineEdit->text();
            ui->chainLabel->setText(ui->chainLabel->text() + "->" + ui->lineEdit->text());
        }
        else
        {
            ui->chainLabel->setText(ui->chainLabel->text() + "+" + ui->lineEdit->text());
            return;
        }
    }

    QString str = ui->lineEdit->text();

    ui->lineEdit_3->setText(item->text());
    ui->lineEdit_2->setText("");

    ui->lineEdit->setText(str);
}

void StructureDescriptor::on_listWidget_itemDoubleClicked(QListWidgetItem *item)
{
    if(ui->recordChainCheckBox->isChecked())
    {
        chain.push_back({chainJoinTable,item->text(),""});
        Qt::KeyboardModifiers modifiers  = QApplication::queryKeyboardModifiers ();
        if(!modifiers.testFlag(Qt::KeyboardModifier::ControlModifier))
        {
            chainJoinTable = ui->lineEdit->text();
            ui->chainLabel->setText(ui->chainLabel->text() + "->" + item->text());
        }
        else
            ui->chainLabel->setText(ui->chainLabel->text() + "+" + item->text());
    }
}

void StructureDescriptor::on_pasteChainButton_pressed()
{
    QMap<QString,QString> aliases;
    for(int i=0;i<chain.size();i++)
    {
        QString alias = "alias";
        alias+= QVariant(i).toString();
        aliases[chain[i].thistable] = alias;

        QString sqlstr = "";

        if(i == 0)
        {
            sqlstr = "select *\nfrom ";
            if(ui->chainPrefixLineEdit->text().size()>0)
                sqlstr += ui->chainPrefixLineEdit->text() + ".";
            sqlstr +=  chain[i].thistable + " " + alias;
        }
        else
        {
            sqlstr = "\ninner join " ;
            if(ui->chainPrefixLineEdit->text().size()>0)
                sqlstr += ui->chainPrefixLineEdit->text() + ".";
            sqlstr += chain[i].thistable + " " + alias ;

            if(chain[i].forceJoinColumn!="")
            {
                sqlstr+= " on ";
                sqlstr+= aliases[chain[i].prevtable] + "." + chain[i].forceJoinColumn + " = " + alias + "." + chain[i].forceJoinColumn ;
            }
            else
            {
                bool first = true;
                for(auto y : loadWind->cd->highlighter->TableColumnDS.data[chain[i].thistable.trimmed()])
                {
                    QString word = y.first.trimmed() +' '+ y.second;
                    if(y.second.trimmed().size()<=0)
                        word = y.first;
                    word = word.trimmed();
                    // ui->listWidget_2->addItem((word));

                    for(auto y : loadWind->cd->highlighter->TableColumnDS.data[chain[i].prevtable.trimmed()])
                    {
                        QString word2 = y.first.trimmed() +' '+ y.second;
                        if(y.second.trimmed().size()<=0)
                            word2 = y.first;
                        word2 = word2.trimmed();
                        if(word == word2)
                        {

                            if(first)
                            {
                                first = false;
                                sqlstr+= " on ";
                            }
                            else
                                sqlstr+= " and ";

                            sqlstr += aliases[chain[i].prevtable] + "." + word+ " = " + alias + "." + word ;



                        }
                    }
                }
            }
        }

        loadWind->cd->textCursor().insertText(sqlstr);
    }

}

void StructureDescriptor::on_clearChainButton_pressed()
{
    chain.clear();
    ui->chainLabel->clear();
}


