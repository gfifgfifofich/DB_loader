#include "docwindow.h"
#include "ui_docwindow.h"

DocWindow::DocWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::DocWindow)
{
    ui->setupUi(this);
    //ui->treeWidget->addTopLevelItem(qtwi );

    ui->treeWidget->setColumnCount(1);
    QList<QTreeWidgetItem *> items;

    items.push_back(new QTreeWidgetItem(static_cast<QTreeWidget*>(nullptr),QStringList("Commands")));
    items.back()->addChild(new QTreeWidgetItem(static_cast<QTreeWidget*>(nullptr),QStringList("execution")));
    items.back()->child(0)->addChild(new QTreeWidgetItem(static_cast<QTreeWidget*>(nullptr),QStringList("SubexecToSqliteTable")));
    items.back()->child(0)->addChild(new QTreeWidgetItem(static_cast<QTreeWidget*>(nullptr),QStringList("SubexecToCSV")));
    items.back()->child(0)->addChild(new QTreeWidgetItem(static_cast<QTreeWidget*>(nullptr),QStringList("SubexecAppendCSV")));
    items.back()->child(0)->addChild(new QTreeWidgetItem(static_cast<QTreeWidget*>(nullptr),QStringList("SubexecToExcelTable")));
    items.back()->child(0)->addChild(new QTreeWidgetItem(static_cast<QTreeWidget*>(nullptr),QStringList("SubexecToExcelWorksheet")));
    items.back()->child(0)->addChild(new QTreeWidgetItem(static_cast<QTreeWidget*>(nullptr),QStringList("SubexecAppendExcelWorksheet")));
    items.back()->child(0)->addChild(new QTreeWidgetItem(static_cast<QTreeWidget*>(nullptr),QStringList("SubexecToMagic")));
    items.back()->child(0)->addChild(new QTreeWidgetItem(static_cast<QTreeWidget*>(nullptr),QStringList("SubexecToArray")));
    items.back()->child(0)->addChild(new QTreeWidgetItem(static_cast<QTreeWidget*>(nullptr),QStringList("SubexecToUnionAllTable")));
    items.back()->child(0)->addChild(new QTreeWidgetItem(static_cast<QTreeWidget*>(nullptr),QStringList("StartAsyncExecution")));
    items.back()->child(0)->addChild(new QTreeWidgetItem(static_cast<QTreeWidget*>(nullptr),QStringList("AwaitAsyncExecution")));
    items.back()->addChild(new QTreeWidgetItem(static_cast<QTreeWidget*>(nullptr),QStringList("fileImport")));
    items.back()->child(1)->addChild(new QTreeWidgetItem(static_cast<QTreeWidget*>(nullptr),QStringList("ExcelToMagic")));
    items.back()->child(1)->addChild(new QTreeWidgetItem(static_cast<QTreeWidget*>(nullptr),QStringList("ExcelToArray")));
    items.back()->child(1)->addChild(new QTreeWidgetItem(static_cast<QTreeWidget*>(nullptr),QStringList("ExcelToSqliteTable")));
    items.back()->child(1)->addChild(new QTreeWidgetItem(static_cast<QTreeWidget*>(nullptr),QStringList("CSVToMagic")));
    items.back()->child(1)->addChild(new QTreeWidgetItem(static_cast<QTreeWidget*>(nullptr),QStringList("CSVToArray")));
    items.back()->child(1)->addChild(new QTreeWidgetItem(static_cast<QTreeWidget*>(nullptr),QStringList("CSVToSqliteTable")));
    items.back()->addChild(new QTreeWidgetItem(static_cast<QTreeWidget*>(nullptr),QStringList("Loops")));
    items.back()->child(2)->addChild(new QTreeWidgetItem(static_cast<QTreeWidget*>(nullptr),QStringList("ForLoop")));
    items.back()->child(2)->addChild(new QTreeWidgetItem(static_cast<QTreeWidget*>(nullptr),QStringList("QueryForLoop")));
    items.back()->addChild(new QTreeWidgetItem(static_cast<QTreeWidget*>(nullptr),QStringList("Misc")));
    items.back()->child(3)->addChild(new QTreeWidgetItem(static_cast<QTreeWidget*>(nullptr),QStringList("SendMail")));

    ui->treeWidget->insertTopLevelItems(0, items);

    ui->treeWidget->setHeaderLabels(QStringList("Name"));
}

DocWindow::~DocWindow()
{
    delete ui;
}

void DocWindow::on_treeWidget_itemChanged(QTreeWidgetItem *item, int column)// selected different doc
{


}


void DocWindow::on_treeWidget_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
    ui->label->setText(current->text(0));

    for(auto x : label_ptrs)
    {
        ui->scrollAreaWidgetContents->layout()->removeWidget(x);
        x->setText("");// if it isnt deleating stuff by itself, at least lower memory usage
    }

    for(auto x : cd_ptrs)
    {
        ui->scrollAreaWidgetContents->layout()->removeWidget(x);
        x->setPlainText("");// if it isnt deleating stuff by itself, at least lower memory usage
    }


    if(current->text(0) == "SubexecToSqliteTable")
    {
        label_ptrs.push_back(new QLabel());
        ui->scrollAreaWidgetContents->layout()->addWidget(label_ptrs.back());
        label_ptrs.back()->setText("A command to execute code in a separate DatabaseConnection, and save result in a table in local database. Soon will be renamed to SubexecToLocalTable");
        label_ptrs.back()->setWordWrap(true);

        label_ptrs.push_back(new QLabel());
        ui->scrollAreaWidgetContents->layout()->addWidget(label_ptrs.back());
        label_ptrs.back()->setText("Parameters:");
        label_ptrs.back()->setWordWrap(true);

        label_ptrs.push_back(new QLabel());
        ui->scrollAreaWidgetContents->layout()->addWidget(label_ptrs.back());
        label_ptrs.back()->setText("Driver - driver wich will be used in connection");
        label_ptrs.back()->setWordWrap(true);

        label_ptrs.push_back(new QLabel());
        ui->scrollAreaWidgetContents->layout()->addWidget(label_ptrs.back());
        label_ptrs.back()->setText("Database - Database in wich the code will be executed");
        label_ptrs.back()->setWordWrap(true);

        label_ptrs.push_back(new QLabel());
        ui->scrollAreaWidgetContents->layout()->addWidget(label_ptrs.back());
        label_ptrs.back()->setText("Tablename - Name of local table. Local table will be replaced, if exists");
        label_ptrs.back()->setWordWrap(true);

        cd_ptrs.push_back(new CodeEditor());
        cd_ptrs.back()->b_codePreview=false;
        cd_ptrs.back()->b_showSuggestion=false;
        ui->scrollAreaWidgetContents->layout()->addWidget(cd_ptrs.back());
        cd_ptrs.back()->setPlainText("SubexecToSqliteTable {--{SQLite} {SQLite.db} {Cool_New_Table} \n\t select 'a' as test\n}");

        label_ptrs.push_back(new QLabel());
        ui->scrollAreaWidgetContents->layout()->addWidget(label_ptrs.back());
        label_ptrs.back()->setText("This code will create a table \"Cool_New_Table\" in a local database, containing a result of this query, e.g a single column \"test\" with a single row");
        label_ptrs.back()->setWordWrap(true);

    }
}

