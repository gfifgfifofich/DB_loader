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
    items.back()->addChild(new QTreeWidgetItem(static_cast<QTreeWidget*>(nullptr),QStringList("Execution")));
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
    items.back()->addChild(new QTreeWidgetItem(static_cast<QTreeWidget*>(nullptr),QStringList("FileImport")));
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
    // fix drawing issues on swap
    // memory leak strategy didnt work at all

    ui->label->setText(current->text(0));

    for(auto x : label_ptrs)
    {
        ui->scrollAreaWidgetContents->layout()->removeWidget(x);
        x->setText("");// if it isnt deleating stuff by itself, at least lower memory usage
        x->deleteLater();
    }
    label_ptrs.clear();
    for(auto x : cd_ptrs)
    {
        ui->scrollAreaWidgetContents->layout()->removeWidget(x);
        x->setPlainText("");// if it isnt deleating stuff by itself, at least lower memory usage
        x->deleteLater();
    }
    cd_ptrs.clear();

    if(current->text(0) == "Commands")
    {
        label_ptrs.push_back(new QLabel());
        ui->scrollAreaWidgetContents->layout()->addWidget(label_ptrs.back());
        label_ptrs.back()->setText(tr("This program adds a few commands to preprocess your sql code and do a little bit of automation. Theese commands highlighter yellow by default. Highlight colors can be changed in settings"));
        label_ptrs.back()->setWordWrap(true);
        label_ptrs.back()->setTextInteractionFlags(Qt::TextSelectableByMouse);

        label_ptrs.push_back(new QLabel());
        ui->scrollAreaWidgetContents->layout()->addWidget(label_ptrs.back());
        label_ptrs.back()->setText(tr("All commands have same syntax, first goes command name, then {} and arguments each in same brackets. the rest of code inside main function brackets will be used or discarded depending on function. (e.g. ForLoop will use it as code to loop)\n\n Example command: "));
        label_ptrs.back()->setWordWrap(true);
        label_ptrs.back()->setTextInteractionFlags(Qt::TextSelectableByMouse);


        cd_ptrs.push_back(new CodeEditor());
        cd_ptrs.back()->b_codePreview=false;
        cd_ptrs.back()->b_showSuggestion=false;
        ui->scrollAreaWidgetContents->layout()->addWidget(cd_ptrs.back());
        cd_ptrs.back()->setPlainText("SubexecToSqliteTable { -- {arg1} {arg2} {arg3}\n\t some stuff\n}");
        cd_ptrs.back()->adjustSize();


        label_ptrs.push_back(new QLabel());
        ui->scrollAreaWidgetContents->layout()->addWidget(label_ptrs.back());
        label_ptrs.back()->setText(tr("All commands are executed from top to bottom, left to right, and can be used inside themselfs. Here is a code do do 5 queries in local database, using forloop and subexec's."));
        label_ptrs.back()->setWordWrap(true);
        label_ptrs.back()->setTextInteractionFlags(Qt::TextSelectableByMouse);


        cd_ptrs.push_back(new CodeEditor());
        cd_ptrs.back()->b_codePreview=false;
        cd_ptrs.back()->b_showSuggestion=false;
        ui->scrollAreaWidgetContents->layout()->addWidget(cd_ptrs.back());
        cd_ptrs.back()->setPlainText("QueryForLoop { --{iter}{0}{5}\n\t SilentSubexecToCSV { -- {LOCAL} {LOCAL} {file_iter}\n\t\t select 'iter' as \"column\" \n\t}\n}");
        cd_ptrs.back()->adjustSize();

    }
    if(current->text(0) == "Execution")
    {

        label_ptrs.push_back(new QLabel());
        ui->scrollAreaWidgetContents->layout()->addWidget(label_ptrs.back());
        label_ptrs.back()->setText(tr("Execution commands are commands that can execute queries and do something with result"));
        label_ptrs.back()->setWordWrap(true);
        label_ptrs.back()->setTextInteractionFlags(Qt::TextSelectableByMouse);

        label_ptrs.push_back(new QLabel());
        ui->scrollAreaWidgetContents->layout()->addWidget(label_ptrs.back());
        label_ptrs.back()->setText(tr("Most theese commands are SubexexSomethingSomething, which are going to return status of execution back to query. folowing code shows an example of such automation with debug information used"));
        label_ptrs.back()->setWordWrap(true);
        label_ptrs.back()->setTextInteractionFlags(Qt::TextSelectableByMouse);


        cd_ptrs.push_back(new CodeEditor());
        cd_ptrs.back()->b_codePreview=false;
        cd_ptrs.back()->b_showSuggestion=false;
        ui->scrollAreaWidgetContents->layout()->addWidget(cd_ptrs.back());
        cd_ptrs.back()->setPlainText("select SubexecToSqliteTable { -- {LOCAL} {LOCAL} {cool_table_a}\n\tselect 'a' as cool_Column \n} "
                                     "\n"
                                     "union all select SubexecToSqliteTable { -- {LOCAL} {LOCAL} {cool_table_b}\n\tselect 'b' as cool_Column \n} \n");
        cd_ptrs.back()->adjustSize();


        label_ptrs.push_back(new QLabel());
        ui->scrollAreaWidgetContents->layout()->addWidget(label_ptrs.back());
        label_ptrs.back()->setText(tr("Most of Subexecs have an analog, with 'Silent' prefix. Theese commands are executed without returning anything to code"));
        label_ptrs.back()->setWordWrap(true);
        label_ptrs.back()->setTextInteractionFlags(Qt::TextSelectableByMouse);


        cd_ptrs.push_back(new CodeEditor());
        cd_ptrs.back()->b_codePreview=false;
        cd_ptrs.back()->b_showSuggestion=false;
        ui->scrollAreaWidgetContents->layout()->addWidget(cd_ptrs.back());
        cd_ptrs.back()->setPlainText("ForLoop { --{iter}{0}{5}\n SilentSubexecToCSV { -- {LOCAL} {LOCAL} {file_iter}\n select 'iter' as \"column\" }}");
        cd_ptrs.back()->adjustSize();
    }
    if(current->text(0) == "FileImport")
    {

        label_ptrs.push_back(new QLabel());
        ui->scrollAreaWidgetContents->layout()->addWidget(label_ptrs.back());
        label_ptrs.back()->setText(tr("FileImport commands are commands to automate file importing"));
        label_ptrs.back()->setWordWrap(true);
        label_ptrs.back()->setTextInteractionFlags(Qt::TextSelectableByMouse);

        label_ptrs.push_back(new QLabel());
        ui->scrollAreaWidgetContents->layout()->addWidget(label_ptrs.back());
        label_ptrs.back()->setText(tr("Most theese commands are SubexexSomethingSomething, which are going to return status of execution back to query. folowing code shows an example of such automation with debug information used"));
        label_ptrs.back()->setWordWrap(true);
        label_ptrs.back()->setTextInteractionFlags(Qt::TextSelectableByMouse);


        cd_ptrs.push_back(new CodeEditor());
        cd_ptrs.back()->b_codePreview=false;
        cd_ptrs.back()->b_showSuggestion=false;
        ui->scrollAreaWidgetContents->layout()->addWidget(cd_ptrs.back());
        cd_ptrs.back()->setPlainText("ExcelToSqliteTable { -- {path/to/excel/file.xlsx} {cool_local_table} }\n"
                                     "select * from cool_local_table");
        cd_ptrs.back()->adjustSize();
    }
    if(current->text(0) == "Loops")
    {
        label_ptrs.push_back(new QLabel());
        ui->scrollAreaWidgetContents->layout()->addWidget(label_ptrs.back());
        label_ptrs.back()->setText(tr("Commands to not type a lot of repetitions, due to SQL not having a basic forloops"));
        label_ptrs.back()->setWordWrap(true);
        label_ptrs.back()->setTextInteractionFlags(Qt::TextSelectableByMouse);

        cd_ptrs.push_back(new CodeEditor());
        cd_ptrs.back()->b_codePreview=false;
        cd_ptrs.back()->b_showSuggestion=false;
        ui->scrollAreaWidgetContents->layout()->addWidget(cd_ptrs.back());
        cd_ptrs.back()->setPlainText("select ForLoop { --{iter}{0}{5}\n]t 'value iter' as \"value iter\" \n}\n ");
        cd_ptrs.back()->adjustSize();
    }


    if(current->text(0) == "ExcelToMagic")
    {
        label_ptrs.push_back(new QLabel());
        ui->scrollAreaWidgetContents->layout()->addWidget(label_ptrs.back());
        label_ptrs.back()->setText(tr("Import excel table, paste first or selected column as oracle-style-ish array: ('magic','data1'),('magic','data2')"));
        label_ptrs.back()->setWordWrap(true);
        label_ptrs.back()->setTextInteractionFlags(Qt::TextSelectableByMouse);
    }
    if(current->text(0) == "ExcelToArray")
    {

    }
    if(current->text(0) == "ExcelToSqliteTable")
    {

    }
    if(current->text(0) == "CSVToMagic")
    {

    }
    if(current->text(0) == "CSVToArray")
    {

    }
    if(current->text(0) == "CSVToSqliteTable")
    {

    }


    if(current->text(0) == "ForLoop")
    {
        label_ptrs.push_back(new QLabel());
        ui->scrollAreaWidgetContents->layout()->addWidget(label_ptrs.back());
        label_ptrs.back()->setText(tr("For loop can be used in c-style, with numbers, or to iterate over array of strings\n\n\n c-style For loop to iterate from 0 to 5 (6 iterations)"));
        label_ptrs.back()->setWordWrap(true);
        label_ptrs.back()->setTextInteractionFlags(Qt::TextSelectableByMouse);

        label_ptrs.push_back(new QLabel());
        ui->scrollAreaWidgetContents->layout()->addWidget(label_ptrs.back());
        label_ptrs.back()->setText(tr("Arguments:\n"
                                   "\n"
                                   "    iterator_value - text that will be replaced in loop body. \n\n"
                                   "    from/list - may be eather a value ({1} or {10} or {023}), or a list of items, separated by comma {item1,item2,item3}.\n\n"
                                   "    to - end loop when iter_value will reach this number."));
        label_ptrs.back()->setWordWrap(true);
        label_ptrs.back()->setTextInteractionFlags(Qt::TextSelectableByMouse);

        cd_ptrs.push_back(new CodeEditor());
        cd_ptrs.back()->b_codePreview=false;
        cd_ptrs.back()->b_showSuggestion=false;
        ui->scrollAreaWidgetContents->layout()->addWidget(cd_ptrs.back());
        cd_ptrs.back()->setPlainText("select ForLoop { --{iter}{0}{5}\nt 'value iter' as \"value iter\" \n}\n ");
        cd_ptrs.back()->adjustSize();


        label_ptrs.push_back(new QLabel());
        ui->scrollAreaWidgetContents->layout()->addWidget(label_ptrs.back());
        label_ptrs.back()->setText(tr("c-style For loop to iterate from 0 to 5 (6 iterations). In this case values will be pasted as 00,01,02,03,04,05... 10,11,12 ect."));
        label_ptrs.back()->setWordWrap(true);
        label_ptrs.back()->setTextInteractionFlags(Qt::TextSelectableByMouse);

        cd_ptrs.push_back(new CodeEditor());
        cd_ptrs.back()->b_codePreview=false;
        cd_ptrs.back()->b_showSuggestion=false;
        ui->scrollAreaWidgetContents->layout()->addWidget(cd_ptrs.back());
        cd_ptrs.back()->setPlainText("select ForLoop { --{iter}{00}{05}\nt 'value iter' as \"value iter\" \n}\n ");
        cd_ptrs.back()->adjustSize();

        label_ptrs.push_back(new QLabel());
        ui->scrollAreaWidgetContents->layout()->addWidget(label_ptrs.back());
        label_ptrs.back()->setText(tr("For loop ovar array of strings:"));
        label_ptrs.back()->setWordWrap(true);
        label_ptrs.back()->setTextInteractionFlags(Qt::TextSelectableByMouse);

        cd_ptrs.push_back(new CodeEditor());
        cd_ptrs.back()->b_codePreview=false;
        cd_ptrs.back()->b_showSuggestion=false;
        ui->scrollAreaWidgetContents->layout()->addWidget(cd_ptrs.back());
        cd_ptrs.back()->setPlainText("select ForLoop { --{iter}{text,textb,text3234,teeeext}\n]t 'value iter' as \"value iter\" \n}\n ");
        cd_ptrs.back()->adjustSize();
    }
    if(current->text(0) == "QueryForLoop")
    {
        label_ptrs.push_back(new QLabel());
        ui->scrollAreaWidgetContents->layout()->addWidget(label_ptrs.back());
        label_ptrs.back()->setText(tr("Works exactly as ForLoop, but will paste 'unian all' between iterations. If a valid query is pasted inside this loop, result will be valid"));
        label_ptrs.back()->setWordWrap(true);
        label_ptrs.back()->setTextInteractionFlags(Qt::TextSelectableByMouse);


        label_ptrs.push_back(new QLabel());
        ui->scrollAreaWidgetContents->layout()->addWidget(label_ptrs.back());
        label_ptrs.back()->setText(tr("Arguments:\n"
                                   "\n"
                                   "    iterator_value - text that will be replaced in loop body. \n\n"
                                   "    from/list - may be eather a value ({1} or {10} or {023}), or a list of items, separated by comma {item1,item2,item3}.\n\n"
                                   "    to - end loop when iter_value will reach this number."));
        label_ptrs.back()->setWordWrap(true);
        label_ptrs.back()->setTextInteractionFlags(Qt::TextSelectableByMouse);


        cd_ptrs.push_back(new CodeEditor());
        cd_ptrs.back()->b_codePreview=false;
        cd_ptrs.back()->b_showSuggestion=false;
        ui->scrollAreaWidgetContents->layout()->addWidget(cd_ptrs.back());
        cd_ptrs.back()->setPlainText("QueryForLoop { --{iter}{0}{5}\nt select  'value iter' as \"value iter\" \n}\n ");
        cd_ptrs.back()->adjustSize();
    }


    if(current->text(0) == "SubexecToSqliteTable")
    {
        label_ptrs.push_back(new QLabel());
        ui->scrollAreaWidgetContents->layout()->addWidget(label_ptrs.back());
        label_ptrs.back()->setText(tr("A command to execute code in a separate DatabaseConnection, and save result in a table in local database. Soon will be renamed to SubexecToLocalTable."));
        label_ptrs.back()->setWordWrap(true);
        label_ptrs.back()->setTextInteractionFlags(Qt::TextSelectableByMouse);

        label_ptrs.push_back(new QLabel());
        ui->scrollAreaWidgetContents->layout()->addWidget(label_ptrs.back());
        label_ptrs.back()->setText(tr("Parameters:"));
        label_ptrs.back()->setWordWrap(true);
        label_ptrs.back()->setTextInteractionFlags(Qt::TextSelectableByMouse);

        label_ptrs.push_back(new QLabel());
        ui->scrollAreaWidgetContents->layout()->addWidget(label_ptrs.back());
        label_ptrs.back()->setText(tr("    Driver - driver wich will be used in connection"));
        label_ptrs.back()->setWordWrap(true);
        label_ptrs.back()->setTextInteractionFlags(Qt::TextSelectableByMouse);

        label_ptrs.push_back(new QLabel());
        ui->scrollAreaWidgetContents->layout()->addWidget(label_ptrs.back());
        label_ptrs.back()->setText(tr("    Database - Database in wich the code will be executed"));
        label_ptrs.back()->setWordWrap(true);
        label_ptrs.back()->setTextInteractionFlags(Qt::TextSelectableByMouse);

        label_ptrs.push_back(new QLabel());
        ui->scrollAreaWidgetContents->layout()->addWidget(label_ptrs.back());
        label_ptrs.back()->setText(tr("    Tablename - Name of local table. Local table will be replaced, if exists"));
        label_ptrs.back()->setWordWrap(true);

        label_ptrs.push_back(new QLabel());
        ui->scrollAreaWidgetContents->layout()->addWidget(label_ptrs.back());
        label_ptrs.back()->setText(tr("This code will create a table \"Cool_New_Table\" in a local database, containing a result of this query, e.g a single column \"test\" with a single row. In this case, code will be executed in sqlite database using sqlite driver"));
        label_ptrs.back()->setWordWrap(true);
        label_ptrs.back()->setTextInteractionFlags(Qt::TextSelectableByMouse);

        cd_ptrs.push_back(new CodeEditor());
        cd_ptrs.back()->b_codePreview=false;
        cd_ptrs.back()->b_showSuggestion=false;
        ui->scrollAreaWidgetContents->layout()->addWidget(cd_ptrs.back());
        cd_ptrs.back()->setPlainText("SubexecToSqliteTable {--{SQLite} {SQLite.db} {Cool_New_Table} \n\t select 'a' as test\n}");
        cd_ptrs.back()->adjustSize();

        label_ptrs.push_back(new QLabel());
        ui->scrollAreaWidgetContents->layout()->addWidget(label_ptrs.back());
        label_ptrs.back()->setText(tr("Exammple code to do preparation of data and then run main query in local database. Database/drivers can be easily replaced"));
        label_ptrs.back()->setWordWrap(true);
        label_ptrs.back()->setTextInteractionFlags(Qt::TextSelectableByMouse);

        cd_ptrs.push_back(new CodeEditor());
        cd_ptrs.back()->b_codePreview=false;
        cd_ptrs.back()->b_showSuggestion=false;
        ui->scrollAreaWidgetContents->layout()->addWidget(cd_ptrs.back());
        cd_ptrs.back()->setPlainText("SubexecToSqliteTable {--{SQLite} {SQLite.db} {Cool_New_Table} "
                                     "\n\t select 'a' as test"
                                     "\n}"
                                     "\n select * from Cool_New_Table");
        cd_ptrs.back()->adjustSize();
    }

    if(current->text(0) == "SubexecToCSV")
    {
        label_ptrs.push_back(new QLabel());
        ui->scrollAreaWidgetContents->layout()->addWidget(label_ptrs.back());
        label_ptrs.back()->setText(tr("A command to execute code in a separate DatabaseConnection, and save result in a .csv file, located in documents/DBLoader/CSV/Filename.csv. Overwrites file if already exists. Will fail to save if file was opened in something like Excel"));
        label_ptrs.back()->setWordWrap(true);
        label_ptrs.back()->setTextInteractionFlags(Qt::TextSelectableByMouse);

        label_ptrs.push_back(new QLabel());
        ui->scrollAreaWidgetContents->layout()->addWidget(label_ptrs.back());
        label_ptrs.back()->setText(tr("Parameters:"));
        label_ptrs.back()->setWordWrap(true);
        label_ptrs.back()->setTextInteractionFlags(Qt::TextSelectableByMouse);

        label_ptrs.push_back(new QLabel());
        ui->scrollAreaWidgetContents->layout()->addWidget(label_ptrs.back());
        label_ptrs.back()->setText(tr("    Driver - driver wich will be used in connection"));
        label_ptrs.back()->setWordWrap(true);
        label_ptrs.back()->setTextInteractionFlags(Qt::TextSelectableByMouse);

        label_ptrs.push_back(new QLabel());
        ui->scrollAreaWidgetContents->layout()->addWidget(label_ptrs.back());
        label_ptrs.back()->setText(tr("    Database - Database in wich the code will be executed"));
        label_ptrs.back()->setWordWrap(true);
        label_ptrs.back()->setTextInteractionFlags(Qt::TextSelectableByMouse);

        label_ptrs.push_back(new QLabel());
        ui->scrollAreaWidgetContents->layout()->addWidget(label_ptrs.back());
        label_ptrs.back()->setText(tr("    Filename - name of file"));
        label_ptrs.back()->setWordWrap(true);
        label_ptrs.back()->setTextInteractionFlags(Qt::TextSelectableByMouse);

        cd_ptrs.push_back(new CodeEditor());
        cd_ptrs.back()->b_codePreview=false;
        cd_ptrs.back()->b_showSuggestion=false;
        ui->scrollAreaWidgetContents->layout()->addWidget(cd_ptrs.back());
        cd_ptrs.back()->setPlainText("SubexecToCSV {--{SQLite} {SQLite.db} {Cool_New_csv} \n\t select 'a' as test\n}");

        label_ptrs.push_back(new QLabel());
        ui->scrollAreaWidgetContents->layout()->addWidget(label_ptrs.back());
        label_ptrs.back()->setText(tr("This code will create a table \"Cool_New_csv.csv\" documents/DBLoader/CSV/Filename.csv, containing a result of this query, e.g a single column \"test\" with a single row"));
        label_ptrs.back()->setWordWrap(true);
        label_ptrs.back()->setTextInteractionFlags(Qt::TextSelectableByMouse);
    }

    if(current->text(0) == "SubexecAppendCSV")
    {
        label_ptrs.push_back(new QLabel());
        ui->scrollAreaWidgetContents->layout()->addWidget(label_ptrs.back());
        label_ptrs.back()->setText(tr("A command to execute code in a separate DatabaseConnection, and save result in a .csv file, located in documents/DBLoader/CSV/Filename.csv. Appends data to file. Will fail to save if file was opened in something like Excel"));
        label_ptrs.back()->setWordWrap(true);
        label_ptrs.back()->setTextInteractionFlags(Qt::TextSelectableByMouse);

        label_ptrs.push_back(new QLabel());
        ui->scrollAreaWidgetContents->layout()->addWidget(label_ptrs.back());
        label_ptrs.back()->setText(tr("Parameters:"));
        label_ptrs.back()->setWordWrap(true);
        label_ptrs.back()->setTextInteractionFlags(Qt::TextSelectableByMouse);

        label_ptrs.push_back(new QLabel());
        ui->scrollAreaWidgetContents->layout()->addWidget(label_ptrs.back());
        label_ptrs.back()->setText(tr("    Driver - driver wich will be used in connection"));
        label_ptrs.back()->setWordWrap(true);
        label_ptrs.back()->setTextInteractionFlags(Qt::TextSelectableByMouse);

        label_ptrs.push_back(new QLabel());
        ui->scrollAreaWidgetContents->layout()->addWidget(label_ptrs.back());
        label_ptrs.back()->setText(tr("    Database - Database in wich the code will be executed"));
        label_ptrs.back()->setWordWrap(true);
        label_ptrs.back()->setTextInteractionFlags(Qt::TextSelectableByMouse);

        label_ptrs.push_back(new QLabel());
        ui->scrollAreaWidgetContents->layout()->addWidget(label_ptrs.back());
        label_ptrs.back()->setText(tr("    Filename - name of file"));
        label_ptrs.back()->setWordWrap(true);
        label_ptrs.back()->setTextInteractionFlags(Qt::TextSelectableByMouse);

        cd_ptrs.push_back(new CodeEditor());
        cd_ptrs.back()->b_codePreview=false;
        cd_ptrs.back()->b_showSuggestion=false;
        ui->scrollAreaWidgetContents->layout()->addWidget(cd_ptrs.back());
        cd_ptrs.back()->setPlainText("SubexecAppendCSV {--{SQLite} {SQLite.db} {Cool_New_csv} \n\t select 'a' as test\n}");

        label_ptrs.push_back(new QLabel());
        ui->scrollAreaWidgetContents->layout()->addWidget(label_ptrs.back());
        label_ptrs.back()->setText(tr("This code will create a table \"Cool_New_csv.csv\" documents/DBLoader/CSV/Filename.csv, containing a result of this query, e.g a single column \"test\" with a single row. If executed multiple times, will add more rows to that file"));
        label_ptrs.back()->setWordWrap(true);
        label_ptrs.back()->setTextInteractionFlags(Qt::TextSelectableByMouse);
    }

    if(current->text(0) == "SubexecToExcelTable")
    {
        label_ptrs.push_back(new QLabel());
        ui->scrollAreaWidgetContents->layout()->addWidget(label_ptrs.back());
        label_ptrs.back()->setText(tr("A command to execute code in a separate DatabaseConnection, and save result in a .xlsx file, located in documents/DBLoader/excel/filename.xlsx. Overwrites file if already exists. Will fail to save if file was opened in something like Excel"));
        label_ptrs.back()->setWordWrap(true);
        label_ptrs.back()->setTextInteractionFlags(Qt::TextSelectableByMouse);

        label_ptrs.push_back(new QLabel());
        ui->scrollAreaWidgetContents->layout()->addWidget(label_ptrs.back());
        label_ptrs.back()->setText(tr("Parameters:"));
        label_ptrs.back()->setWordWrap(true);
        label_ptrs.back()->setTextInteractionFlags(Qt::TextSelectableByMouse);

        label_ptrs.push_back(new QLabel());
        ui->scrollAreaWidgetContents->layout()->addWidget(label_ptrs.back());
        label_ptrs.back()->setText(tr("    Driver - driver wich will be used in connection"));
        label_ptrs.back()->setWordWrap(true);
        label_ptrs.back()->setTextInteractionFlags(Qt::TextSelectableByMouse);

        label_ptrs.push_back(new QLabel());
        ui->scrollAreaWidgetContents->layout()->addWidget(label_ptrs.back());
        label_ptrs.back()->setText(tr("    Database - Database in wich the code will be executed"));
        label_ptrs.back()->setWordWrap(true);
        label_ptrs.back()->setTextInteractionFlags(Qt::TextSelectableByMouse);

        label_ptrs.push_back(new QLabel());
        ui->scrollAreaWidgetContents->layout()->addWidget(label_ptrs.back());
        label_ptrs.back()->setText(tr("    Filename - name of file"));
        label_ptrs.back()->setWordWrap(true);
        label_ptrs.back()->setTextInteractionFlags(Qt::TextSelectableByMouse);

        cd_ptrs.push_back(new CodeEditor());
        cd_ptrs.back()->b_codePreview=false;
        cd_ptrs.back()->b_showSuggestion=false;
        ui->scrollAreaWidgetContents->layout()->addWidget(cd_ptrs.back());
        cd_ptrs.back()->setPlainText("SubexecToExcelTable {--{SQLite} {SQLite.db} {Cool_New_xlsx} \n\t select 'a' as test\n}");

        label_ptrs.push_back(new QLabel());
        ui->scrollAreaWidgetContents->layout()->addWidget(label_ptrs.back());
        label_ptrs.back()->setText(tr("This code will create a table \"Cool_New_xlsx.xlsx\" in documents/DBLoader/excel/filename.xlsx, containing a result of this query, e.g a single column \"test\" with a single row"));
        label_ptrs.back()->setWordWrap(true);
        label_ptrs.back()->setTextInteractionFlags(Qt::TextSelectableByMouse);
    }

    if(current->text(0) == "SubexecToExcelWorksheet")
    {
        label_ptrs.push_back(new QLabel());
        ui->scrollAreaWidgetContents->layout()->addWidget(label_ptrs.back());
        label_ptrs.back()->setText(tr("A command to execute code in a separate DatabaseConnection, and save result in a .xlsx file, located in documents/DBLoader/excel/filename.xlsx. Overwrites file if already exists. Will fail to save if file was opened in something like Excel"));
        label_ptrs.back()->setWordWrap(true);
        label_ptrs.back()->setTextInteractionFlags(Qt::TextSelectableByMouse);

        label_ptrs.push_back(new QLabel());
        ui->scrollAreaWidgetContents->layout()->addWidget(label_ptrs.back());
        label_ptrs.back()->setText(tr("Parameters:"));
        label_ptrs.back()->setWordWrap(true);
        label_ptrs.back()->setTextInteractionFlags(Qt::TextSelectableByMouse);

        label_ptrs.push_back(new QLabel());
        ui->scrollAreaWidgetContents->layout()->addWidget(label_ptrs.back());
        label_ptrs.back()->setText(tr("    Driver - driver wich will be used in connection"));
        label_ptrs.back()->setWordWrap(true);
        label_ptrs.back()->setTextInteractionFlags(Qt::TextSelectableByMouse);

        label_ptrs.push_back(new QLabel());
        ui->scrollAreaWidgetContents->layout()->addWidget(label_ptrs.back());
        label_ptrs.back()->setText(tr("    Database - Database in wich the code will be executed"));
        label_ptrs.back()->setWordWrap(true);
        label_ptrs.back()->setTextInteractionFlags(Qt::TextSelectableByMouse);

        label_ptrs.push_back(new QLabel());
        ui->scrollAreaWidgetContents->layout()->addWidget(label_ptrs.back());
        label_ptrs.back()->setText(tr("    Filename - name of file"));
        label_ptrs.back()->setWordWrap(true);
        label_ptrs.back()->setTextInteractionFlags(Qt::TextSelectableByMouse);

        label_ptrs.push_back(new QLabel());
        ui->scrollAreaWidgetContents->layout()->addWidget(label_ptrs.back());
        label_ptrs.back()->setText(tr("    SheetName - name of Sheet in excel table"));
        label_ptrs.back()->setWordWrap(true);
        label_ptrs.back()->setTextInteractionFlags(Qt::TextSelectableByMouse);

        cd_ptrs.push_back(new CodeEditor());
        cd_ptrs.back()->b_codePreview=false;
        cd_ptrs.back()->b_showSuggestion=false;
        ui->scrollAreaWidgetContents->layout()->addWidget(cd_ptrs.back());
        cd_ptrs.back()->setPlainText("SubexecToExcelWorksheet {--{SQLite} {SQLite.db} {Cool_New_xlsx} \n\t select 'a' as test\n}");

        label_ptrs.push_back(new QLabel());
        ui->scrollAreaWidgetContents->layout()->addWidget(label_ptrs.back());
        label_ptrs.back()->setText(tr("This code will create a table \"Cool_New_xlsx.xlsx\" in documents/DBLoader/excel/filename.xlsx, containing a result of this query, e.g a single column \"test\" with a single row. Data will be located on a sheet \"SheetName\", overwriting all data on that list. If file contained pivottables or other info, it will be deleated, use SubexecAppendExcelWorksheet{}."));
        label_ptrs.back()->setWordWrap(true);
        label_ptrs.back()->setTextInteractionFlags(Qt::TextSelectableByMouse);
    }

    if(current->text(0) == "SubexecAppendExcelWorksheet")
    {
        label_ptrs.push_back(new QLabel());
        ui->scrollAreaWidgetContents->layout()->addWidget(label_ptrs.back());
        label_ptrs.back()->setText(tr("A command to execute code in a separate DatabaseConnection, and save result in a .xlsx file, located in documents/DBLoader/excel/filename.xlsx. Overwrites file if already exists. Will fail to save if file was opened in something like Excel"));
        label_ptrs.back()->setWordWrap(true);
        label_ptrs.back()->setTextInteractionFlags(Qt::TextSelectableByMouse);

        label_ptrs.push_back(new QLabel());
        ui->scrollAreaWidgetContents->layout()->addWidget(label_ptrs.back());
        label_ptrs.back()->setText(tr("Parameters:"));
        label_ptrs.back()->setWordWrap(true);
        label_ptrs.back()->setTextInteractionFlags(Qt::TextSelectableByMouse);

        label_ptrs.push_back(new QLabel());
        ui->scrollAreaWidgetContents->layout()->addWidget(label_ptrs.back());
        label_ptrs.back()->setText(tr("    Driver - driver wich will be used in connection"));
        label_ptrs.back()->setWordWrap(true);
        label_ptrs.back()->setTextInteractionFlags(Qt::TextSelectableByMouse);

        label_ptrs.push_back(new QLabel());
        ui->scrollAreaWidgetContents->layout()->addWidget(label_ptrs.back());
        label_ptrs.back()->setText(tr("    Database - Database in wich the code will be executed"));
        label_ptrs.back()->setWordWrap(true);
        label_ptrs.back()->setTextInteractionFlags(Qt::TextSelectableByMouse);

        label_ptrs.push_back(new QLabel());
        ui->scrollAreaWidgetContents->layout()->addWidget(label_ptrs.back());
        label_ptrs.back()->setText(tr("    Filename - name of file"));
        label_ptrs.back()->setWordWrap(true);
        label_ptrs.back()->setTextInteractionFlags(Qt::TextSelectableByMouse);

        label_ptrs.push_back(new QLabel());
        ui->scrollAreaWidgetContents->layout()->addWidget(label_ptrs.back());
        label_ptrs.back()->setText(tr("    SheetName - name of Sheet in excel table"));
        label_ptrs.back()->setWordWrap(true);
        label_ptrs.back()->setTextInteractionFlags(Qt::TextSelectableByMouse);

        cd_ptrs.push_back(new CodeEditor());
        cd_ptrs.back()->b_codePreview=false;
        cd_ptrs.back()->b_showSuggestion=false;
        ui->scrollAreaWidgetContents->layout()->addWidget(cd_ptrs.back());
        cd_ptrs.back()->setPlainText("SubexecAppendExcelWorksheet {--{SQLite} {SQLite.db} {Cool_New_xlsx} \n\t select 'a' as test\n}");

        label_ptrs.push_back(new QLabel());
        ui->scrollAreaWidgetContents->layout()->addWidget(label_ptrs.back());
        label_ptrs.back()->setText(tr("This code will create a table \"Cool_New_xlsx.xlsx\" in documents/DBLoader/excel/filename.xlsx, containing a result of this query, e.g a single column \"test\" with a single row. Data will be located on a sheet \"SheetName\", overwriting all data on that list. new Data will be placed below other data on the list. Somewhat scuffed due to use of ODBC_Excel driver, but data is safe"));
        label_ptrs.back()->setWordWrap(true);
        label_ptrs.back()->setTextInteractionFlags(Qt::TextSelectableByMouse);
    }

    if(current->text(0) == "SubexecToMagic")
    {
        label_ptrs.push_back(new QLabel());
        ui->scrollAreaWidgetContents->layout()->addWidget(label_ptrs.back());
        label_ptrs.back()->setText(tr("A command to execute code in a separate DatabaseConnection, and paste result in a oracle-specific format of arrays to allow more rows beeng put into 'in()' statement format: ('magic','data1'),('magic','data2')"));
        label_ptrs.back()->setWordWrap(true);
        label_ptrs.back()->setTextInteractionFlags(Qt::TextSelectableByMouse);

        label_ptrs.push_back(new QLabel());
        ui->scrollAreaWidgetContents->layout()->addWidget(label_ptrs.back());
        label_ptrs.back()->setText(tr("Parameters:"));
        label_ptrs.back()->setWordWrap(true);
        label_ptrs.back()->setTextInteractionFlags(Qt::TextSelectableByMouse);

        label_ptrs.push_back(new QLabel());
        ui->scrollAreaWidgetContents->layout()->addWidget(label_ptrs.back());
        label_ptrs.back()->setText(tr("    Driver - driver wich will be used in connection"));
        label_ptrs.back()->setWordWrap(true);
        label_ptrs.back()->setTextInteractionFlags(Qt::TextSelectableByMouse);

        label_ptrs.push_back(new QLabel());
        ui->scrollAreaWidgetContents->layout()->addWidget(label_ptrs.back());
        label_ptrs.back()->setText(tr("    Database - Database in wich the code will be executed"));
        label_ptrs.back()->setWordWrap(true);
        label_ptrs.back()->setTextInteractionFlags(Qt::TextSelectableByMouse);

        cd_ptrs.push_back(new CodeEditor());
        cd_ptrs.back()->b_codePreview=false;
        cd_ptrs.back()->b_showSuggestion=false;
        ui->scrollAreaWidgetContents->layout()->addWidget(cd_ptrs.back());
        cd_ptrs.back()->setPlainText("select * from table_with_ids where ('magic',id) in (SubexecToMagic {--{SQLite} {SQLite.db} \n\t select id from important_Local_table\n})");

        label_ptrs.push_back(new QLabel());
        ui->scrollAreaWidgetContents->layout()->addWidget(label_ptrs.back());
        label_ptrs.back()->setText(tr("This is a crude way to awoid dblinks, or get data depending on external sources."));
        label_ptrs.back()->setWordWrap(true);
        label_ptrs.back()->setTextInteractionFlags(Qt::TextSelectableByMouse);
    }

    if(current->text(0) == "SubexecToArray")
    {
        label_ptrs.push_back(new QLabel());
        ui->scrollAreaWidgetContents->layout()->addWidget(label_ptrs.back());
        label_ptrs.back()->setText(tr("A command to execute code in a separate DatabaseConnection, and paste result in standart-ish sql array format: 'data1', 'data2', 'data3' "));
        label_ptrs.back()->setWordWrap(true);
        label_ptrs.back()->setTextInteractionFlags(Qt::TextSelectableByMouse);

        label_ptrs.push_back(new QLabel());
        ui->scrollAreaWidgetContents->layout()->addWidget(label_ptrs.back());
        label_ptrs.back()->setText(tr("Parameters:"));
        label_ptrs.back()->setWordWrap(true);
        label_ptrs.back()->setTextInteractionFlags(Qt::TextSelectableByMouse);

        label_ptrs.push_back(new QLabel());
        ui->scrollAreaWidgetContents->layout()->addWidget(label_ptrs.back());
        label_ptrs.back()->setText(tr("    Driver - driver wich will be used in connection"));
        label_ptrs.back()->setWordWrap(true);
        label_ptrs.back()->setTextInteractionFlags(Qt::TextSelectableByMouse);

        label_ptrs.push_back(new QLabel());
        ui->scrollAreaWidgetContents->layout()->addWidget(label_ptrs.back());
        label_ptrs.back()->setText(tr("    Database - Database in wich the code will be executed"));
        label_ptrs.back()->setWordWrap(true);
        label_ptrs.back()->setTextInteractionFlags(Qt::TextSelectableByMouse);

        cd_ptrs.push_back(new CodeEditor());
        cd_ptrs.back()->b_codePreview=false;
        cd_ptrs.back()->b_showSuggestion=false;
        ui->scrollAreaWidgetContents->layout()->addWidget(cd_ptrs.back());
        cd_ptrs.back()->setPlainText("select * from table_with_ids where id in (SubexecToArray {--{SQLite} {SQLite.db} \n\t select id from important_Local_table\n})");

        label_ptrs.push_back(new QLabel());
        ui->scrollAreaWidgetContents->layout()->addWidget(label_ptrs.back());
        label_ptrs.back()->setText(tr("This is a crude way to awoid dblinks, or get data depending on external sources."));
        label_ptrs.back()->setWordWrap(true);
        label_ptrs.back()->setTextInteractionFlags(Qt::TextSelectableByMouse);
    }

    if(current->text(0) == "SubexecToUnionAllTable")
    {
        label_ptrs.push_back(new QLabel());
        ui->scrollAreaWidgetContents->layout()->addWidget(label_ptrs.back());
        label_ptrs.back()->setText(tr("A command to execute code in a separate DatabaseConnection, and paste result in a bunch of select 'a' as \"columnname\" union all select 'b' as \"columnname\" "));
        label_ptrs.back()->setWordWrap(true);
        label_ptrs.back()->setTextInteractionFlags(Qt::TextSelectableByMouse);

        label_ptrs.push_back(new QLabel());
        ui->scrollAreaWidgetContents->layout()->addWidget(label_ptrs.back());
        label_ptrs.back()->setText(tr("Parameters:"));
        label_ptrs.back()->setWordWrap(true);
        label_ptrs.back()->setTextInteractionFlags(Qt::TextSelectableByMouse);

        label_ptrs.push_back(new QLabel());
        ui->scrollAreaWidgetContents->layout()->addWidget(label_ptrs.back());
        label_ptrs.back()->setText(tr("    Driver - driver wich will be used in connection"));
        label_ptrs.back()->setWordWrap(true);
        label_ptrs.back()->setTextInteractionFlags(Qt::TextSelectableByMouse);

        label_ptrs.push_back(new QLabel());
        ui->scrollAreaWidgetContents->layout()->addWidget(label_ptrs.back());
        label_ptrs.back()->setText(tr("    Database - Database in wich the code will be executed"));
        label_ptrs.back()->setWordWrap(true);
        label_ptrs.back()->setTextInteractionFlags(Qt::TextSelectableByMouse);

        cd_ptrs.push_back(new CodeEditor());
        cd_ptrs.back()->b_codePreview=false;
        cd_ptrs.back()->b_showSuggestion=false;
        ui->scrollAreaWidgetContents->layout()->addWidget(cd_ptrs.back());
        cd_ptrs.back()->setPlainText("with cool_table as (SubexecToUnionAllTable{-- {LOCAL} {LOCAL} \nselect * from tmp\n}) \n select * from cool_table");

        label_ptrs.push_back(new QLabel());
        ui->scrollAreaWidgetContents->layout()->addWidget(label_ptrs.back());
        label_ptrs.back()->setText(tr("most Databases are wery slow with this type of actions"));
        label_ptrs.back()->setWordWrap(true);
        label_ptrs.back()->setTextInteractionFlags(Qt::TextSelectableByMouse);
    }

    if(current->text(0) == "StartAsyncExecution")
    {
        label_ptrs.push_back(new QLabel());
        ui->scrollAreaWidgetContents->layout()->addWidget(label_ptrs.back());
        label_ptrs.back()->setText(tr("Do not use without AwaitAsyncExecution{}"));
        label_ptrs.back()->setWordWrap(true);
        label_ptrs.back()->setTextInteractionFlags(Qt::TextSelectableByMouse);

        label_ptrs.push_back(new QLabel());
        ui->scrollAreaWidgetContents->layout()->addWidget(label_ptrs.back());
        label_ptrs.back()->setText(tr("All subexecs after this command and 'AwaitAsyncExecution{}' will be launched in parallel"));
        label_ptrs.back()->setWordWrap(true);
        label_ptrs.back()->setTextInteractionFlags(Qt::TextSelectableByMouse);



        label_ptrs.push_back(new QLabel());
        ui->scrollAreaWidgetContents->layout()->addWidget(label_ptrs.back());
        label_ptrs.back()->setText(tr("This example will launch 5 subexecs in local database in parallel: "));
        label_ptrs.back()->setWordWrap(true);
        label_ptrs.back()->setTextInteractionFlags(Qt::TextSelectableByMouse);

        cd_ptrs.push_back(new CodeEditor());
        cd_ptrs.back()->b_codePreview=false;
        cd_ptrs.back()->b_showSuggestion=false;
        ui->scrollAreaWidgetContents->layout()->addWidget(cd_ptrs.back());
        cd_ptrs.back()->setPlainText("StartAsyncExecution {--{}} \nForLoop { --{iter}{0}{5}\n SilentSubexecToCSV { -- {LOCAL} {LOCAL} {file_iter} select 'iter' as \"column\" }}\n AwaitAsyncExecution{--{}}");
        cd_ptrs.back()->adjustSize();

    }
    if(current->text(0) == "AwaitAsyncExecution")
    {
        label_ptrs.push_back(new QLabel());
        ui->scrollAreaWidgetContents->layout()->addWidget(label_ptrs.back());
        label_ptrs.back()->setText(tr("A command to wait for all async subexecs to finish, and process what their commands (save to table/ paste stuff, ect.)"));
        label_ptrs.back()->setWordWrap(true);
        label_ptrs.back()->setTextInteractionFlags(Qt::TextSelectableByMouse);


    }

}

