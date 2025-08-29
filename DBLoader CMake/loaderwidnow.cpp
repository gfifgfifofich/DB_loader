#include "loaderwidnow.h"
#include "structuredescriptor.h"
#include "ui_loaderwidnow.h"
#include <qaxobject.h>
#include <qbarset.h>
#include <qclipboard.h>
#include <qcommandlineparser.h>
#include <qdir.h>
#include <qfilesystemmodel.h>
#include <qmimedata.h>
#include <qpdfwriter.h>
#include <qrandom.h>
#include <qshortcut.h>
#include <fstream>
#include <qsqldriver.h>
#include <qsqlerror.h>
#include <QDesktopServices>
#include <QFileDialog>
#include "replacedialog.h"
#include <QQmlApplicationEngine>
#include <qstylefactory.h>
#include <qtimer.h>
#include "tokenprocessor.h"
#include <QInputDialog>
#include <qtreeview.h>
#include "settingswindow.h"
#include "include/SimpleMail/SimpleMail"
#include <QDockWidget>
#include "docwindow.h"
#include <QInputDialog>
#include <QMessageBox>

/*
+                                          add togglable "add db name into file name" // feature added, not togglable
+                                          add ability to stop downloading query at any point of downloading, mb togglable autopause at 500
+                                          save only driver and db name in workspaces, fetch the rest from userdata, its safer
+                                          themes from userdata
+                                          Replace window - replaces every "string" with other "string", yea
+                                          Highlight from bracket to bracket with codeEditor HighLight selection (will highlight background from bracket to bracket)
+                                          Highlighting of selected token
+-                                         fix subtables
+                                          datatypes
+                                          tab press reg
+                                          Timer
- replace data in excel file using id column
- replace excel worksheet
- Fix append (create if not exists)
- Subexec as text
+- reimplement Patterns compleatly through .ds files

+ open files through cmd
+ Scroll code preview


    ui->tabWidget->tabText(ui->tabWidget->currentIndex());
    ui->tabWidget->setTabText(ui->tabWidget->currentIndex(),"asdasda");
on_workspaceLineEdit_textChanged(ui->tabWidget->tabText(ui->tabWidget->currentIndex()));



*/


inline QDateTime autolaunchLastLaunch;
inline QString usrDir;
inline QString documentsDir;
inline DataStorage userDS;
inline DataStorage historyDS;

inline DataStorage uniqueTokens;
inline std::map<int,QString> uniqueTokenIdMap;

inline QString appfilename;
inline int thrnum;
inline QTime lastMultiRunPress = QTime::currentTime();

GlContext gl_compute_context;
ComputeShader gl_compute_shader;
ComputeShader gl_compute_shader_output_error;
ComputeShader gl_compute_shader_backpropagate_hidden;
ComputeShader gl_compute_shader_update_weights_biases;

QStringList allPosibbleTokens;
QMap<QString,int> allPosibbleTokensMap;

void asyncLoadFrequencyMapFunc(QString filename)
{

    qDebug() << "loading token processor";
    //
    //load tokenprocessor data
    if(!tokenProcessor.ds.Load(filename))
        qDebug() << "failed to load FrequencyMap: " <<  filename;



    // tokenProcessor.uniqueTokens.clear();
    // for(auto x : tokenProcessor.ds.data)
    //     for(auto y : x.second)
    //     {
    //         // tokenProcessor.ds.SetProperty(x.first,y.first,y.second);
    //         tokenProcessor.uniqueTokens[y.first] = y.first;
    //     }

    // QFile fl ((documentsDir + "/" +"FrequencyMaps/tokens.txt"));
    // fl.open(QFile::OpenModeFlag::WriteOnly);
    // int i =0;
    // fl.write("tokens\n");
    // fl.write("{\n");

    // for(auto a : tokenProcessor.uniqueTokens)
    // {
    //     fl.write((" "+ a  + " " + QVariant(i).toString() + "\n").toUtf8());
    //     i++;
    // }
    // fl.write("}\n");
    // fl.close();


    qDebug() << "loaded token processor";
}

LoaderWidnow::LoaderWidnow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::LoaderWidnow)
{
    ui->setupUi(this);
    ui->stopLoadingQueryButton->hide();




    // setup compute shader stuff
    //Context - for create and maintaing OpenGL context
    gl_compute_context.setup();

    //Compute shader
    QString shader_file = "./shader/compute_shader.csh";
    QString shader_file_output_error = "./shader/compute_shader_output_error.csh";
    QString shader_file_backpropagate_hidden = "./shader/compute_shader_backpropagate_hidden.csh";
    QString shader_file_update_weights_biases = "./shader/compute_shader_update_weights_biases.csh";

    gl_compute_shader.setup(shader_file, &gl_compute_context);
    gl_compute_shader_output_error.setup(shader_file_output_error, &gl_compute_context);
    gl_compute_shader_backpropagate_hidden.setup(shader_file_backpropagate_hidden, &gl_compute_context);
    gl_compute_shader_update_weights_biases.setup(shader_file_update_weights_biases, &gl_compute_context);

    // Remove old shader loading
    // QString shader_file_learn = "./shader/compute_shader_learn.csh";
    // gl_compute_shader_learn.setup(shader_file_learn, &gl_compute_context);

    // QString shader_file_rank = "./shader/compute_shader_rank.csh";
    // gl_compute_shader_rank.setup(shader_file_rank, &gl_compute_context);



    ui->connectionStatusLabel_2->setTextInteractionFlags(Qt::TextSelectableByMouse);

    // global pointer to ease modifications of this window from others (find, replace and other tools)
    loadWind  = this;

    // accept drag and drop
    ui->tableWidget->setAcceptDrops(true);
    this->setAcceptDrops(true);

    // graph window init
    gw.Init();
    ui->graph_layout->addLayout(&gw.graph_layout);

    // set progressbar to 0
    ui->exportProgressBar->setMaximum(1);
    ui->exportProgressBar->setValue(0);

    //hide autolauncher/automation
    ui->timerdayMonthly->hide();
    ui->timerDayWeekly->hide();
    ui->timerHourWeekly->hide();
    ui->timerHourDaily->hide();
    ui->timerHourMonthly->hide();
    ui->timerMinuteDaily->hide();
    ui->timerMinuteWeekly->hide();
    ui->timerMinuteMonthly->hide();
    ui->timer_checkBox->hide();
    ui->timer_checkBox_2->hide();
    ui->timer_checkBox_3->hide();
    ui->timerMainLabel->hide();
    ui->timerRemainingTime->hide();
    ui->timerLastLaunchTime->hide();
    autolaunchLastLaunch = QDateTime::currentDateTime();
    autolaunchTimer.setInterval(1000);
    autolaunchTimer.setSingleShot(false);
    autolaunchTimer.start();

    //workspace setup. Should be reimplemented due to crash on windows on file deleation from file explorer when app is running. FileSystemModel does the crash
    workspace_model = new QFileSystemModel();
    workspace_tree = new QTreeView();
    workspace_model->setRootPath(documentsDir);
    workspace_tree->setModel(workspace_model);
    workspace_tree->setRootIndex(workspace_model->index(documentsDir));
    workspace_tree->setAnimated(true);
    workspace_tree->setIndentation(20);
    workspace_tree->setSortingEnabled(true);
    workspace_tree->setSelectionMode(QAbstractItemView::SingleSelection);
    const QSize availableSize = workspace_tree->screen()->availableGeometry().size();
    workspace_tree->resize(availableSize / 2);
    workspace_tree->setColumnWidth(0, workspace_tree->width() / 3);

    ui->workspace_layout->addWidget(workspace_tree);

    // hide workspaces/history
    workspace_tree->hide();





    // init tabs, first code editor instance, DataConnection
    while (ui->tabWidget->tabBar()->count()>0)
    {
        ui->tabWidget->removeTab(0);
    }

    QWidget* wg = new QWidget();
    wg->setWhatsThis(QVariant(0).toString());

    ui->tabWidget->addTab(wg,"New tab" + QVariant(0).toString());
    ui->tabWidget->setTabWhatsThis(ui->tabWidget->tabBar()->count()-1,QVariant(0).toString());

    tabDatas.push_back(new tabdata());
    tabDatas.back()->Name = "New tab0";
    tabDatas.back()->Id = "0";

    tabDatas.back()->cd = new CodeEditor();

    cd = tabDatas.back()->cd;
    ui->edit_layout->addWidget(cd);

    dc = &tabDatas.back()->dc;
    sqlexecThread = &tabDatas.back()->sqlexecThread;



    // QDockWidget* dw = new QDockWidget();

    // ui->edit_layout->addWidget(dw);



    //// Menubar Actions, additional keyboard shortcuts



    //open documentation window
    connect(ui->actionsubfunction_docs,  &QAction::triggered, this, [this]() {
        DocWindow* dw = new DocWindow();
        dw->show();
    });

    //open new app instance
    connect(ui->actionNew_window,  &QAction::triggered, this, [this]() {
        OpenNewWindow();
    });

    //open new Tab
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_N), this, SLOT(on_pushButton_4_clicked()));

    // run query
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_R), this, SLOT(runSqlAsync()));
    connect(ui->actionRun,  &QAction::triggered, this, [this]() {
        runSqlAsync();
    });

    //.xlsx export
    connect(ui->actionSave_as_excel,  &QAction::triggered, this, [this]() {
        on_SaveXLSXButton_pressed();
    });

    // open .xlsx file
    connect(ui->actionOpen_last_excel_export,  &QAction::triggered, this, [this]() {
        OpenFile();
    });
    // open .xlsx dir
    connect(ui->actionOpen_export_directory,  &QAction::triggered, this, [this]() {
        OpenDirectory();
    });

    // save workspace
    connect(ui->actionSave_workspace,  &QAction::triggered, this, [this]() {
        SaveWorkspace();
    });

    // replace code
    connect(ui->actionReplace,  &QAction::triggered, this, [this]() {
        replaceTool();
    });
    // comment code
    connect(ui->actionCommentSelected,  &QAction::triggered, this, [this]() {
        CommentSelected();
    });

    /// Other actions
    // Split column code
    connect(ui->actionSplit_column,  &QAction::triggered, this, [this]() {
        splitColumn();
    });
    // Add numerator column
    connect(ui->actionAdd_Numerator_column,  &QAction::triggered, this, [this]() {
        addNumeratorColumn();
    });
    // Add numerator column
    connect(ui->actionDelete_dublicates,  &QAction::triggered, this, [this]() {
        deleteDublicates();
    });

    // Add numerator column
    connect(ui->actionLevenstein_join,  &QAction::triggered, this, [this]() {
        levensteinJoin();
    });




    // copy from tableview
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_C), this, SLOT(CopySelectionFormTable()));
    new QShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_C), this, SLOT(CopySelectionFormTableSql()));

    // switching different windosheet
    connect(ui->actionGraph,  &QAction::triggered, this, [this]() {
        ShowGraph();
    });
    connect(ui->actionHistory,  &QAction::triggered, this, [this]() {
        ShowHistoryWindow();
    });
    connect(ui->actionWorkspaces,  &QAction::triggered, this, [this]() {
        ShowWorkspacesWindow();
    });
    connect(ui->actionAutorun,  &QAction::triggered, this, [this]() {
        ShowTimerWindow();
    });

    new QShortcut(QKeySequence(Qt::CTRL | 96), this, SLOT(cycleTabs())); // ctrl + ` || ctrl + ~ || ctrl + Ё

    connect(ui->actionRun_token_processor,  &QAction::triggered, this, [this]() {
        on_pushButton_2_pressed();
    });
    connect(ui->actionOpen_QML_form,  &QAction::triggered, this, [this]() {
        on_pushButton_pressed();
    });
    connect(ui->actionNN_Run,  &QAction::triggered, this, [this]() {
        on_nnTestRun_pressed();
    });
    connect(ui->actionNN_Learn_first_column,  &QAction::triggered, this, [this]() {
        on_nnTestLearn_pressed();
    });
    connect(ui->actionCopy_last_launched_sql,  &QAction::triggered, this, [this]() {
        CopyLastLaunchedSql();
    });

    connect(ui->actionUserTheme, &QAction::triggered, this, [this]() {
        SettingsWindow* st = new SettingsWindow();
        connect(st, SIGNAL(saved()), this, SLOT(updateMisc()), Qt::QueuedConnection );
        st->show();
    });


    connect(ui->actionDescriber, &QAction::triggered, this, [this]() {
        openDescriptorWindow();
    });



    //signal binding
    //query states
    connect( dc, SIGNAL(queryBeginCreating()), this, SLOT(onQueryBeginCreating()), Qt::QueuedConnection );
    connect( dc, SIGNAL(queryBeginExecuting()), this, SLOT(onQueryBegin()), Qt::QueuedConnection );
    connect( dc, SIGNAL(querySuccess()), this, SLOT(onQuerySuccess()), Qt::QueuedConnection );
    connect( dc, SIGNAL(execedSql()), this, SLOT(UpdateTable()), Qt::QueuedConnection );

    connect( &dc->data, SIGNAL(ExportedToCSV()), this, SLOT(on_exportDone()), Qt::QueuedConnection );
    connect( &dc->data, SIGNAL(ExportedToExcel()), this, SLOT(on_exportDone()), Qt::QueuedConnection );
    connect( &dc->data, SIGNAL(ExportedToSQLiteTable()), this, SLOT(on_exportDone()), Qt::QueuedConnection );


    connect( dc, SIGNAL(sendMail(QString, QString, QString, QStringList,QStringList, QString, QString, QStringList, QStringList)), this, SLOT(sendMail(QString, QString, QString, QStringList,QStringList, QString, QString, QStringList, QStringList)), Qt::QueuedConnection );
    connect( dc, SIGNAL(saveGraph(DatabaseConnection*)), this, SLOT(saveGraph(DatabaseConnection*)), Qt::BlockingQueuedConnection );

    // autoexecution
    connect(&executionTimer, SIGNAL(timeout()), this, SLOT(executionTimerTimeout()));
    connect(&autolaunchTimer, SIGNAL(timeout()), this, SLOT(autolaunchCheck()));

    //codeeditor
    connect( cd, SIGNAL(s_suggestedName()), this, SLOT(updatesuggestion()), Qt::QueuedConnection );
    //graph
    connect( &gw.buildGraphButton, SIGNAL(pressed()), this, SLOT(UpdateGraph()), Qt::QueuedConnection );
    connect( &gw.saveAsPDFButton, SIGNAL(pressed()), this, SLOT(saveGraphAsPDF()), Qt::QueuedConnection );
    connect( &gw.copyScriptHandle, SIGNAL(pressed()), this, SLOT(copyGraphScriptHandle()), Qt::QueuedConnection );
    connect( &gw.groupBysb, SIGNAL(valueChanged(int)), this, SLOT(on_graph_group_change(int)), Qt::QueuedConnection );
    connect( &gw.separateBysb, SIGNAL(valueChanged(int)), this, SLOT(on_graph_separator_change(int)), Qt::QueuedConnection );
    connect( &gw.dataColumnsb, SIGNAL(valueChanged(int)), this, SLOT(on_graph_data_change(int)), Qt::QueuedConnection );


    connect( ui->tableWidget, SIGNAL(ui->tableWidget->verticalHeader()->sectionDoubleClicked(int)),this, SLOT(tableHeaderDoubleClicked(int)), Qt::QueuedConnection );

    //workspace/history change
    //connect( workspace_tree, SIGNAL(activated(QModelIndex&)), this, SLOT(on_TreeItem_Changed(QModelIndex&)), Qt::QueuedConnection );
    QItemSelectionModel *selectionModel = workspace_tree->selectionModel();
    connect(selectionModel, &QItemSelectionModel::selectionChanged, this, &LoaderWidnow::on_TreeItem_Changed);



    //maximize code editor
    ui->splitter->setSizes({1,2000});




    ui->CodeEditorSplitter->setSizes({0,2000});

    // load last database/driver
    if(userDS.Load((documentsDir + "/userdata.txt")))
    {
        qDebug() << "Loaded ds:" <<(documentsDir + "/userdata.txt");
        QStringList strl;
        for( auto x : userDS.data["UserDBs"])
        {
            strl.push_back((x.first + " " +  x.second));
            strl.back() = strl.back().trimmed();
        }
        ui->DBNameComboBox->addItems(strl);
        ui->DBNameComboBox->setCurrentText(userDS.GetProperty("User","lastDBName"));
        strl.clear();
        for( auto x : userDS.data["UserDrivers"])
        {
            strl.push_back((x.second));
            strl.back() = strl.back().trimmed();
        }
        ui->driverComboBox->addItems(strl);
        ui->driverComboBox->setCurrentText(userDS.GetProperty("User","lastDriver"));

        if(QString(userDS.data["UserTheme"]["Workspace_Directory"]).trimmed().size() < 3)
            userDS.data["UserTheme"]["Workspace_Directory"] = "documents";
        qDebug() << "saved 1";

    }


    // if program was opened from command line with filename to open
    if(launchOpenFile)
    {
        QFile file(launchOpenFileName);
        if (file.open(QFile::ReadOnly | QFile::Text))
            cd->setPlainText(file.readAll());
        LastWorkspaceName = launchOpenFileName;
        ui->tabWidget->setTabText(ui->tabWidget->currentIndex(),LastWorkspaceName);
        on_workspaceLineEdit_textChanged(ui->tabWidget->tabText(ui->tabWidget->currentIndex()));
        ui->splitter->setSizes({0,2000,0});
    }
    else // open with all the slow stuff, in full edit mode
    {
        if(!LastWorkspaceName.endsWith(".sql"))
            LastWorkspaceName+=".sql";

        ui->tabWidget->setTabText(ui->tabWidget->currentIndex(),LastWorkspaceName);
        on_workspaceLineEdit_textChanged(ui->tabWidget->tabText(ui->tabWidget->currentIndex()));

        QString prefix = documentsDir;
        if(QString(userDS.data["UserTheme"]["Workspace_Directory"]).trimmed() != "documents")
            prefix = QString(userDS.data["UserTheme"]["Workspace_Directory"]).trimmed();

        QFile file(prefix + "/" +"workspaces/" + LastWorkspaceName);
        if (file.open(QFile::ReadOnly | QFile::Text))
            cd->setPlainText(file.readAll());


    }

    if(launchAutomation)
    {
        if(launchIntervalParameter == "daily")
        {
            ui->timer_checkBox->setChecked(true);
            ui->timerHourDaily->setValue(launchIntervalHour.toInt());
            ui->timerMinuteDaily->setValue(launchIntervalMinute.toInt());
        }
    }


    // get database from file header
    QString text = cd->toPlainText();
    if(text.startsWith("-- {"))
    {
        QStringList tokens;
        int i = 0;
        bool inbrakets = false;
        while(i < text.size())
        {
            if(text[i] == '{')
            {
                tokens.push_back("");
                inbrakets = true;
                i++;
                continue;
            }
            if(text[i] == '}')
            {
                tokens.back() = tokens.back().trimmed();
                inbrakets = true;
                if(tokens.size() == 2)
                    break;
                i++;
                continue;
            }
            if(inbrakets)
            {
                tokens.back().push_back(text[i]);
            }
            i++;

        }
        if(tokens.size() >= 2)
        {
            ui->driverComboBox->setCurrentText(tokens[0].trimmed());
            ui->DBNameComboBox->setCurrentText(tokens[1].trimmed());

        }
    }

    // grep name/password
    QString username = userDS.GetObject( ui->DBNameComboBox->currentText())["name"];
    QString password = userDS.GetObject(ui->DBNameComboBox->currentText())["password"];

    // set focus on code editor
    cd->setFocus();


    // load this big boy async, cuz it takes a hot minute to do so
    QThread* thr = QThread::create(asyncLoadFrequencyMapFunc,(documentsDir + "/" +"FrequencyMaps/test.txt"));
    thr->start();

    // connect to database
    // on_ConnectButton_pressed();


    // nn setup. Currently used to learn first column of data, repeat + extrapolate on run

    // QFile fl ((documentsDir + "/" +"FrequencyMaps/tokens.txt"));
    // uniqueTokens.Load((documentsDir + "/" +"FrequencyMaps/asdzxcasd.txt"));
    uniqueTokens.Load((documentsDir + "/" +"FrequencyMaps/tokens.txt"));

    // uniqueTokens.Save((documentsDir + "/" +"FrequencyMaps/tokenstest.txt"));

    qDebug() << uniqueTokens.data["tokens"].size() << "uniqueTokens.data[\"tokens\"].size()";

    int arch[3] = {1,500,1};
    // arch[0] = uniqueTokens.data["tokens"].size();
    // arch[2] = uniqueTokens.data["tokens"].size();

    nn.CreateGPU(arch,3);
    nn.lastCost = 10000000;

    for (int i=0; i<nn.Weights_Size; i++) {
        nn.weights[i] = 0.0f;
    }
    nn.Randomize();
    nn.InitGPUBuffers();


    nn.backPropagateShader = new ComputeShader();
    nn.backPropagateShader->setup("./shader/compute_shader_backpropagate.csh", nn.glContext);
    //nn.LoadFrom((documentsDir + "/nntest_RunTest.nn").toStdString());
}


LoaderWidnow::~LoaderWidnow()
{
    qDebug()<<"closing window";

    _tab_failsave=false;


    // a mess to have more chances of actually terminating all db connections
    for(int i=0;i < tabDatas.size();i++)
    {
        dc = &tabDatas[i]->dc;
        sqlexecThread = &tabDatas[i]->sqlexecThread;
        if(dc!=nullptr)
        {
            try
            {
                dc->stopNow = true;
                if(dc->executing)
                    dc->stopRunning();
                if(dc->executing)
                    dc->stopRunning();
            } catch (...){}
        }

    }
    qDebug()<<"closing tabs";
    int tmpcnt = ui->tabWidget->tabBar()->count();
    while (ui->tabWidget->tabBar()->count()>0)
    {
        on_tabWidget_tabCloseRequested(0);
        if(tmpcnt == ui->tabWidget->tabBar()->count())
        {
            ui->tabWidget->removeTab(0);

            tmpcnt = ui->tabWidget->tabBar()->count();
        }
    }

    delete ui;
}


void LoaderWidnow::tableHeaderDoubleClicked(int id)
{
    qDebug() << "clicked on "<< id << ui->tableWidget->verticalHeaderItem(id)->text();
}

inline int _dbconnectcount = 0;
void LoaderWidnow::on_ConnectButton_pressed()
{
    qDebug()<<"on_ConnectButton_pressed()";

    //tooo slow and absolutely pointless to do on every reconnect
    // if(!tokenProcessor.ds.Load((documentsDir + "/" +"FrequencyMaps/test.txt")))
    //     qDebug() << "failed to load FrequencyMap: " <<  (documentsDir + "/" +"FrequencyMaps/test.txt");


    // get data from fields
    QString conname = QVariant(_dbconnectcount++).toString();
    QString driver = ui->driverComboBox->currentText();
    QString dbname = ui->DBNameComboBox->currentText();
    QString usrname = ui->userNameLineEdit->text();
    QString password = ui->passwordLineEdit->text();

    //if local db selected, swap all data for local database, in which the save goes
    if(driver.trimmed() == "LOCAL" || dbname.trimmed() == "LOCAL")
    {
        driver = userDS.data["UserTheme"]["db_drv_Save_table_driver"];
        dbname = userDS.data["UserTheme"]["db_drv_Save_table_Connection"];
        dbname = dbname.replace("documentsDir",documentsDir);

        driver = driver.trimmed();
        dbname = dbname.trimmed();

        usrname = userDS.data[dbname]["name"];
        password = userDS.data[dbname]["password"];
        usrname = usrname.trimmed();
        password = password.trimmed();
    }
    dc->disableSaveToUserDS=false;

    dc->connectionName = conname;// set new connection name for all the QT 'native' drivers.

    if(!dc->executing && dc->Create(driver, dbname, usrname, password))
    {

        ui->driverComboBox->setCurrentText(dc->driver);
        ui->DBNameComboBox->setCurrentText(dc->dbname);
        ui->userNameLineEdit->setText(dc->usrname);
        ui->passwordLineEdit->setText(dc->password);
        ui->connectionStatusLabel_2->setText(QString("connected to ") + dc->dbname);

        if(userDS.Load(documentsDir + "/userdata.txt"))
        {
            QString LastTmpDriverName =  ui->driverComboBox->currentText();
            QString LastTmpDbName = ui->DBNameComboBox->currentText();

            userDS.data[ui->DBNameComboBox->currentText()]["name"] = dc->usrname;
            userDS.data[ui->DBNameComboBox->currentText()]["password"] = dc->password;
            userDS.data["User"]["lastDriver"] = dc->driver;
            userDS.data["User"]["lastDBName"] = dc->dbname;
            userDS.data["User"]["name"] = dc->usrname;
            userDS.data["User"]["password"] = dc->password;

            QStringList strl;
            userDS.data["UserDBs"][ui->DBNameComboBox->currentText()];
            for( auto x : userDS.data["UserDBs"])
            {
                strl.push_back((x.first + " " + x.second));
                strl.back() = strl.back().trimmed();
            }
            ui->DBNameComboBox->clear();
            ui->DBNameComboBox->addItems(strl);

            strl.clear();

            for( auto x : userDS.data["UserDrivers"])
            {
                strl.push_back(x.second);
                strl.back() = strl.back().trimmed();
            }

            if(!strl.contains(ui->driverComboBox->currentText()))
                userDS.data["UserDrivers"][QVariant(ui->driverComboBox->count()).toString()] = ui->driverComboBox->currentText();
            ui->driverComboBox->clear();
            ui->driverComboBox->addItems(strl);

            ui->DBNameComboBox->setCurrentText( LastTmpDbName );
            ui->driverComboBox->setCurrentText( LastTmpDriverName );


            userDS.data[ui->DBNameComboBox->currentText()]["name"] = dc->usrname;
            userDS.data[ui->DBNameComboBox->currentText()]["password"] = dc->password;
            userDS.data["User"]["lastDriver"] = driver;
            userDS.data["User"]["lastDBName"] = dc->dbname;
            userDS.data["User"]["name"] = dc->usrname;
            userDS.data["User"]["password"] = dc->password;
            userDS.data["UserDBs"][ui->DBNameComboBox->currentText()] = "";

            userDS.Save((documentsDir + "/userdata.txt"));
            qDebug()<<"Saved usedata.txt";
        }
        else
            qDebug()<<"Failed to open usedata.txt";

        QString str = "";
        if(dbname.split('/').size()>1)
            str = dbname.split('/')[1];
        else str =dbname.split('/')[0];

        cd->highlighter->QSLiteStyle = dc->sqlite || driver == "SQLite";//(driver == "QSQLite") || (driver == "LOCAL_SQLITE_DB");
        cd->highlighter->PostgresStyle = dc->postgre || dc->customPSQL;//(driver == "QPSQL");
        cd->highlighter->UpdateTableColumns(&dc->db,str);

        cd->updateMisc();

    cd->highlighter->updateAllHighlighting = true;
    cd->highlighter->rehighlight();
    cd->highlighter->updateAllHighlighting = false;

        ui->driverComboBox->setCurrentText(dc->driver);
        ui->DBNameComboBox->setCurrentText(dc->dbname);
        ui->userNameLineEdit->setText(dc->usrname);
        ui->passwordLineEdit->setText(dc->password);
    }
    else
        ui->connectionStatusLabel_2->setText("Not connected: " + dc->Last_ConnectError.trimmed());


}

// function to give to thread to process query async
void _AsyncFunc(LoaderWidnow* loader)
{
    loader->dc->execSql();
}

// Query processing
void LoaderWidnow::on_pushButton_3_pressed()
{
    runSqlAsync();
}
void LoaderWidnow::runSqlAsync()
{
    qDebug()<<"RunSqlAsync()";

    if( dc->data.exporting || dc->executing)
    {
        return;
    }
    cd->setFocus();
    ui->stopLoadingQueryButton->show();



    QString conname = QVariant(_dbconnectcount++).toString();
    QString driver = ui->driverComboBox->currentText();
    QString dbname = ui->DBNameComboBox->currentText();
    QString usrname = ui->userNameLineEdit->text();
    QString password = ui->passwordLineEdit->text();
    if(driver != dc->driver || dc->dbname!=dbname || usrname != dc->usrname || password != dc->password)
    {
        on_ConnectButton_pressed();
    }

    // return if executing
    if(dc->executing)
    {

        qDebug() << dc->db.driver()->cancelQuery();
        ui->stopLoadingQueryButton->hide();
        return;
    }

    // hide run button
    ui->pushButton_3->hide();
    queryExecutionState = 0;
    dc->queryExecutionState = 0;

    dc->executing = true;

    // cut part of code to execute
    int sqlstart = 0;
    int sqlend = cd->toPlainText().size();
    if(cd->textCursor().selectedText().size() <= 5)
    {
        dc->sqlCode = "";
        QString text = cd->toPlainText();
        int start = cd->textCursor().position()-1;
        if(start<0)
            start = 0;
        while(start>0 && text[start]!=';')
            start--;
        while(start< text.size() && !text[start].isLetterOrNumber() && text[start] != '-')
            start++;
        QTextCursor tc = cd->textCursor();
        tc.setPosition(start,QTextCursor::MoveAnchor);
        dc->_code_start_line = tc.blockNumber();
        dc->_code_start_pos = tc.position();
        int iter = start;
        sqlstart = start;
        dc->sqlCode.push_back(text[iter]);
        iter++;
        while(iter<text.size())
        {
            dc->sqlCode.push_back(text[iter]);
            sqlend = start;
            if((text[iter]==';'))
                break;
            iter++;
        }
        dc->_code_end_pos = iter;
        tc.setPosition(iter,QTextCursor::MoveAnchor);
        qDebug()<<"sql start "<<start;
        qDebug()<<"sql start "<<iter-1;
        while(dc->sqlCode.endsWith(';'))
            dc->sqlCode.resize(dc->sqlCode.size()-1);
    }
    else
    {
        dc->sqlCode = cd->textCursor().selectedText();
        dc->sqlCode = dc->sqlCode.replace('\r','\n');
        dc->sqlCode = dc->sqlCode.replace(QChar(0x2029), QChar('\n'));

        QTextCursor tc = cd->textCursor();
        tc.setPosition(cd->textCursor().selectionStart(),QTextCursor::MoveAnchor);
        dc->_code_start_line = tc.blockNumber();
        dc->_code_start_pos = tc.position();
    }


    qDebug()<<"Executing sql:";
    ui->miscStatusLabel->setText(QString("running sql subqueries... start: ") + dc->executionStart.toString());
    qDebug()<<dc->sqlCode;
    qDebug()<<"";

    // save sqlBacup
    QString str = "sqlBackup/";
    QDate dt = QDate::currentDate();
    str += QVariant(dt.year()).toString();
    str += "_";
    if(QVariant(dt.month() ).toString().size() <=1)
        str+="0";
    str += QVariant(dt.month()).toString();
    str += "_";
    if(QVariant(dt.day() ).toString().size() <=1)
        str+="0";
    str += QVariant(dt.day() ).toString();
    str += "_";
    str += QTime::currentTime().toString();
    str +=".sql";
    str.replace(":","_");
    qDebug()<<documentsDir + "/" +str;
    std::ofstream stream ((documentsDir + "/" +str).toStdString());
    stream << cd->toPlainText().toStdString();
    stream.close();
    SaveWorkspace();

    // move cursor to end of code, to highlight some user errors with missing ;
    QTextCursor cursor = cd->textCursor();
    cursor.setPosition(sqlstart, QTextCursor::MoveAnchor);
    cursor.setPosition(sqlend+1, QTextCursor::KeepAnchor);
    if((*sqlexecThread)!=nullptr)
        (*sqlexecThread)->terminate();

    // reconnect if needed
    if((!dc->db.isOpen() && !dc->customOracle && !dc->customSQlite && !dc->customPSQL) || dc->driver != ui->driverComboBox->currentText() || dc->dbname != ui->DBNameComboBox->currentText())
    {
        qDebug() << "autocreating connection";
        on_ConnectButton_pressed();
        qDebug() << "autocreated connection";
    }

    //launch thread with query
    ui->miscStatusLabel->setText("running sql...");
    (*sqlexecThread) = QThread::create(_AsyncFunc,this);
    (*sqlexecThread)->start();
    ui->miscStatusLabel->setText("running sql...");

    executionTimer.setSingleShot(false);
    executionTimer.setTimerType(Qt::CoarseTimer);
    executionTimer.setInterval(15);
    executionTimer.start();

}

//timer for autolaunching queries
void LoaderWidnow::autolaunchCheck()
{

    ui->timerLastLaunchTime->setText( "Last launch was " + autolaunchLastLaunch.toString());

    if(ui->timer_checkBox->isChecked())
    {// daily

        QDateTime nextdt = QDateTime::currentDateTime();
        nextdt.setTime(QTime(ui->timerHourDaily->value(), ui->timerMinuteDaily->value()));

        if(QDateTime::currentDateTime().time().minute() > ui->timerMinuteDaily->value() && QDateTime::currentDateTime().time().hour() >= ui->timerHourDaily->value())
        {
            nextdt = nextdt.addDays(1);
        }



        QString dtStr = QVariant(QDateTime::currentDateTime().secsTo(nextdt)%60).toString();
        if(dtStr.size() == 1)
            dtStr = "0" + dtStr;
        dtStr = ":" + dtStr;
        dtStr = QVariant(QDateTime::currentDateTime().secsTo(nextdt)/60%60).toString() + dtStr;
        if(dtStr.size() == 4)
            dtStr = "0" + dtStr;
        dtStr = ":" + dtStr;
        dtStr = QVariant(QDateTime::currentDateTime().secsTo(nextdt)/3600%24).toString() + dtStr;
        if(dtStr.size() == 6)
            dtStr = "0" + dtStr;

        ui->timerRemainingTime->setText("Next launch at:  " + dtStr);

        if( QDateTime::currentDateTime().time().minute() == ui->timerMinuteDaily->value() && QDateTime::currentDateTime().time().hour()== ui->timerHourDaily->value())
        {
            if(abs(QDateTime::currentDateTime().toSecsSinceEpoch() - autolaunchLastLaunch.toSecsSinceEpoch()) >= 60)
            {
                qDebug() << "Launching automaticly due to daily timer";
                autolaunchLastLaunch = QDateTime::currentDateTime();
                runSqlAsync();
            }

        }
    }
    else if(ui->timer_checkBox_3->isChecked())
    {// weekly



        QDateTime nextdt = QDateTime::currentDateTime();
        nextdt.setTime(QTime(ui->timerHourWeekly->value(), ui->timerMinuteWeekly->value()));


        if(QDateTime::currentDateTime().date().dayOfWeek() > ui->timerDayWeekly->value() || QDateTime::currentDateTime().date().dayOfWeek() == ui->timerDayWeekly->value() && QDateTime::currentDateTime().time().minute() > ui->timerMinuteWeekly->value() && QDateTime::currentDateTime().time().hour() >= ui->timerHourWeekly->value())
        {
            nextdt = nextdt.addDays(7 - QDateTime::currentDateTime().date().dayOfWeek() + ui->timerDayWeekly->value() - 1);
        }
        else if(QDateTime::currentDateTime().date().dayOfWeek() < ui->timerDayWeekly->value() && QDateTime::currentDateTime().time().minute() > ui->timerMinuteWeekly->value() && QDateTime::currentDateTime().time().hour() >= ui->timerHourWeekly->value())
        {
            while(nextdt.date().dayOfWeek() > 1)
                nextdt = nextdt.addDays(-1);
            nextdt = nextdt.addDays(ui->timerDayWeekly->value() -2 );
        }

        qint64 tm= QDateTime::currentDateTime().secsTo(nextdt);
        if(tm < 0 && !(tm >-60 && abs(QDateTime::currentDateTime().toSecsSinceEpoch() - autolaunchLastLaunch.toSecsSinceEpoch()) >= 60))
            tm = 86400 + tm;

        QString dtStr = QVariant(tm%60).toString();
        if(dtStr.size() == 1)
            dtStr = "0" + dtStr;
        dtStr = ":" + dtStr;
        dtStr = QVariant(tm/60%60).toString() + dtStr;
        if(dtStr.size() == 4)
            dtStr = "0" + dtStr;
        dtStr = ":" + dtStr;
        dtStr = QVariant(tm/3600%24).toString() + dtStr;
        if(dtStr.size() == 6)
            dtStr = "0" + dtStr;



        ui->timerRemainingTime->setText("Next launch at:  " + QVariant(QDateTime::currentDateTime().daysTo(nextdt)).toString() + ":" + dtStr);

        if( QDateTime::currentDateTime().time().minute() == ui->timerMinuteWeekly->value() && QDateTime::currentDateTime().time().hour()== ui->timerHourWeekly->value() && QDateTime::currentDateTime().date().dayOfWeek() == ui->timerDayWeekly->value())
        {
            if(QDateTime::currentDateTime().toSecsSinceEpoch() - autolaunchLastLaunch.toSecsSinceEpoch() >= 60)
            {
                qDebug() << "Launching automaticly due to weekly timer";
                autolaunchLastLaunch = QDateTime::currentDateTime();
                runSqlAsync();
            }
        }
    }
    else if(ui->timer_checkBox_2->isChecked())
    {// monthly
        QDateTime nextdt = QDateTime::currentDateTime();
        nextdt.setTime(QTime(ui->timerHourMonthly->value(), ui->timerMinuteMonthly->value()));

        if(QDateTime::currentDateTime().date().day() > ui->timerdayMonthly->value() || QDateTime::currentDateTime().date().day() == ui->timerdayMonthly->value() && QDateTime::currentDateTime().time().minute() > ui->timerMinuteMonthly->value() && QDateTime::currentDateTime().time().hour() >= ui->timerHourMonthly->value())
        {
            nextdt = nextdt.addMonths(1);
            while(nextdt.date().day() > 1)
                nextdt = nextdt.addDays(-1);

            nextdt = nextdt.addDays(ui->timerdayMonthly->value()-1);

        }
        else if (QDateTime::currentDateTime().date().day() < ui->timerdayMonthly->value())
        {

            while(nextdt.date().day() > 1)
                nextdt = nextdt.addDays(-1);

            nextdt = nextdt.addDays(ui->timerdayMonthly->value()-2);
        }


        qint64 tm= QDateTime::currentDateTime().secsTo(nextdt);
        if(tm < 0 && !(tm >-60 && abs(QDateTime::currentDateTime().toSecsSinceEpoch() - autolaunchLastLaunch.toSecsSinceEpoch()) >= 60))
            tm = 86400 + tm;

        QString dtStr = QVariant(tm%60).toString();
        if(dtStr.size() == 1)
            dtStr = "0" + dtStr;
        dtStr = ":" + dtStr;
        dtStr = QVariant(tm/60%60).toString() + dtStr;
        if(dtStr.size() == 4)
            dtStr = "0" + dtStr;
        dtStr = ":" + dtStr;
        dtStr = QVariant(tm/3600%24).toString() + dtStr;
        if(dtStr.size() == 6)
            dtStr = "0" + dtStr;


        ui->timerRemainingTime->setText("Next launch at:  " + QVariant(QDateTime::currentDateTime().daysTo(nextdt)).toString() + ":" + dtStr);

        if( QDateTime::currentDateTime().time().minute() == ui->timerMinuteMonthly->value() && QDateTime::currentDateTime().time().hour()== ui->timerHourMonthly->value() && QDateTime::currentDateTime().date().day() == ui->timerdayMonthly->value())
        {
            qDebug() << "Launching automaticly due to monthly timer";
            if(QDateTime::currentDateTime().toSecsSinceEpoch() - autolaunchLastLaunch.toSecsSinceEpoch() >= 60)
            {
                autolaunchLastLaunch = QDateTime::currentDateTime();
                runSqlAsync();
            }
        }
    }
    if(ui->realTimeUpdatecheckBox->isChecked() && (autolaunchLastLaunch.time().msecsTo( QDateTime::currentDateTime().time())>60000))
    {
        autolaunchLastLaunch = QDateTime::currentDateTime();
        runSqlAsync();
    }

}

void LoaderWidnow::executionTimerTimeout()
{// update label

    if(!dc->executing && dc->data.exporting)
    {//exporting data;
        ui->exportProgressBar->show();
        ui->miscStatusLabel->show();

        ui->miscStatusLabel->setText("Exporting: " + QTime::fromMSecsSinceStartOfDay( dc->data.LastSaveDuration.msecsTo(QTime::currentTime())).toString() + "." +  QVariant(dc->data.LastSaveDuration.msecsTo(QTime::currentTime())).toString());

        if(dc->data.saveRowSize>0)
            ui->exportProgressBar->setMaximum(dc->data.saveRowSize);
        else
            ui->exportProgressBar->setMaximum(1);

        ui->exportProgressBar->setValue(dc->data.saveRowsDone);

        return;
    }


    if(dc->dataDownloading && dc->postgre) // since it has ability to count rows, why not show them
    {
        if(dc->data.saveRowSize>0)
            ui->exportProgressBar->setMaximum(dc->data.saveRowSize);
        else
            ui->exportProgressBar->setMaximum(1);
        ui->exportProgressBar->setValue(dc->data.saveRowsDone);
    }
    else
    {
        if(dc->data.tbldata.size()>0 && dc->data.tbldata[0].size() > 0 )
            ui->exportProgressBar->setMaximum(dc->data.tbldata[0].size());
        else
            ui->exportProgressBar->setMaximum(1);
        ui->exportProgressBar->setValue(0);
    }

    QString msg = "";
    if(dc->queryExecutionState >=3)
    {
        msg += QVariant(dc->data.tbldata.size()).toString();
        msg += " : ";
        if(dc->data.tbldata.size()>0)
            msg += QVariant(dc->data.tbldata[0].size()).toString();
        else
            msg += "0";
    }

    if(!dc->executing)
        msg+= " data connection is not executing! ";
    dc->executionTime = QDateTime::currentSecsSinceEpoch() - dc->executionStart.toSecsSinceEpoch();
    dc->executionEnd = QDateTime::currentDateTime();
    QString hours =  QVariant((dc->executionTime / 3600)).toString();
    QString minuts = QVariant(dc->executionTime % 3600 / 60).toString();
    QString secs = QVariant(dc->executionTime % 60).toString();
    QString msecs = QVariant((QDateTime::currentMSecsSinceEpoch() - dc->executionStart.toMSecsSinceEpoch())%1000).toString();

    while(hours.size() <2)
        hours = QString("0") + hours;
    while(minuts.size() <2)
        minuts = QString("0") + minuts;
    while(secs.size() <2)
        secs = QString("0") + secs;
    while(msecs.size() <3)
        msecs = QString("0") + msecs;
    if(dc->queryExecutionState == 0)
        msg += " running subqueries: ";
    else if(dc->queryExecutionState == 1)
        msg += " creating sql query: ";
    else if(dc->queryExecutionState == 2)
        msg += " executing sql query: ";
    else if(dc->queryExecutionState == 3)
        msg += " query success, downloading result: ";
    msg += hours;
    msg += ":";
    msg += minuts;
    msg += ":";
    msg += secs;
    msg += ".";
    msg += msecs;

    msg += "   Successfull queries: ";
    msg += QVariant(dc->savefilecount).toString();
    ui->miscStatusLabel->setText(msg);
}
void LoaderWidnow::onQueryBeginCreating()
{
    qDebug()<<"onQueryBeginCreating()";
    ui->miscStatusLabel->setText(QString("creating sql query...start: ") + dc->executionStart.toString());
    queryExecutionState = 1;
    ui->pushButton_3->hide();
    //ui->stopLoadingQueryButton->hide();
}
void LoaderWidnow::onQueryBegin()
{
    qDebug()<<"onQueryBegin()";
    ui->miscStatusLabel->setText(QString("executing sql query...start: ") + dc->executionStart.toString());
    queryExecutionState = 2;
    ui->pushButton_3->hide();
    //ui->stopLoadingQueryButton->hide();
}
void LoaderWidnow::onQuerySuccess()
{
    qDebug()<<"onQuerySuccess()";
    ui->miscStatusLabel->setText(QString("query success, downloading...start: ") + dc->executionStart.toString());
    queryExecutionState = 3;
    ui->stopLoadingQueryButton->show();
    ui->pushButton_3->hide();
}
void LoaderWidnow::UpdateTable()
{
    // clear tableData, fill tableView with new data up to  25k rosheet, update info label
    qDebug() << "Updating table";
    if((dc->executing || dc->dataDownloading || dc->data.tbldata.size() <1 || dc->data.headers.size() <1) && !dc->lastLaunchIsError)
    {
        ui->tableWidget->clear();
        ui->tableWidget->setColumnCount(0);
        ui->tableWidget->setRowCount(0);
        return;

    }
    if(dc->data.tbldata.size()>0 && dc->data.tbldata[0].size() > 0 )
        ui->exportProgressBar->setMaximum(dc->data.tbldata[0].size());
    else
        ui->exportProgressBar->setMaximum(1);
    ui->exportProgressBar->setValue(0);

    dc->queryExecutionState = 4;
    queryExecutionState = 4;
    executionTimer.stop();



    if(dc->lastLaunchIsError)
    {
        int errpos = dc->_code_start_pos + dc->lastErrorPos;
        if(dc->_code_end_pos-1  < errpos)
        {
            errpos = dc->_code_end_pos-1;
        }
        qDebug() << "errpos = " << dc->_code_start_pos + dc->lastErrorPos << " clamped to"<<errpos ;
        QTextCursor tc = cd->textCursor();
        tc.setPosition(errpos);
        tc.select(QTextCursor::WordUnderCursor);
        cd->setTextCursor(tc);
        cd->setFocus();

    }


    ui->stopLoadingQueryButton->hide();
    ui->pushButton_3->show();
    ui->miscStatusLabel->setText("updating table data...");
    ui->tableWidget->clear();
    ui->tableWidget->setColumnCount(dc->data.headers.size());


    int tabl_size = 25000;
    if(dc->data.tbldata.size()>0)
        tabl_size  = dc->data.tbldata[0].size();

    if(tabl_size  > 25000)
        ui->tableWidget->setRowCount(25000);
    else
        ui->tableWidget->setRowCount(tabl_size );

    ui->tableWidget->setHorizontalHeaderLabels(dc->data.headers);
    for(int i=0;i<dc->data.tbldata.size();i++)
    {
        for (int a=0; (a<dc->data.tbldata[i].size() && a < 25000);a++)
        {
            QTableWidgetItem *item = new QTableWidgetItem(dc->data.tbldata[i][a]);
            ui->tableWidget->setItem(a, i, item);
        }
    }

    ui->tableWidget->resizeColumnsToContents();

    QString msg = "";
    msg += QVariant(dc->data.tbldata.size()).toString();
    msg += " : ";
    if(dc->data.tbldata.size()>0)
        msg += QVariant(dc->data.tbldata[0].size()).toString();
    else
        msg += "0";

    QString hours =  QVariant((dc->executionTime / 3600)).toString();
    QString minuts = QVariant(dc->executionTime % 3600 / 60).toString();
    QString secs = QVariant(dc->executionTime % 60).toString();
    QString msecs = QVariant((dc->executionEnd.toMSecsSinceEpoch() - dc->executionStart.toMSecsSinceEpoch())%1000).toString();

    while(hours.size() <2)
        hours = QString("0") + hours;
    while(minuts.size() <2)
        minuts = QString("0") + minuts;
    while(secs.size() <2)
        secs = QString("0") + secs;
    while(msecs.size() <3)
        msecs = QString("0") + msecs;

    msg += "   execution time: ";
    msg += hours;
    msg += ":";
    msg += minuts;
    msg += ":";
    msg += secs;
    msg += ".";
    msg += msecs;
    msg += "   Successfull queries: ";
    msg += QVariant(dc->savefilecount).toString();
    ui->miscStatusLabel->setText(QString("data downloaded ") + msg);
    ui->dataSizeLabel_2->setText(msg);
    ui->tableDBNameLabel->setText(dc->dbname);

    on_graph_group_change(gw.groupBysb.value());
    on_graph_separator_change(gw.separateBysb.value());
    on_graph_data_change(gw.dataColumnsb.value());
    emit TableUpdated();


    if(ui->updateGraphCheckBox->isChecked())
        UpdateGraph();
}

void LoaderWidnow::OpenDirectory()
{
    qDebug()<<"OpenDirectory()";
    QString str= documentsDir + "/" +"excel/";
    qDebug()<<"opening directory: " << str;
    if(QDesktopServices::openUrl(QUrl::fromLocalFile(str)))
        ui->miscStatusLabel->setText("Opening directory.");
    else
        ui->miscStatusLabel->setText("error opening directory");
}
void LoaderWidnow::OpenFile()
{
    qDebug()<<"OpenFile()";
    QString str= documentsDir + "/" +"excel/";
    str += userDS.data["ExcelPrefixAliases"][cd->highlighter->dbSchemaName];
    str+= ui->saveLineEdit->text();
    if(!str.endsWith(".xlsx"))
        str+= ".xlsx";
    qDebug()<<"opening:" << str;
    if(QDesktopServices::openUrl(QUrl::fromLocalFile(str)))
        ui->miscStatusLabel->setText("Opening file.");
    else
        ui->miscStatusLabel->setText("error opening file.");
}
void LoaderWidnow::OpenNewWindow()
{
    qDebug()<<"OpenNewWindow() ";
    QString str= appfilename;
    qDebug()<<"opening: " << str;
    QDesktopServices::openUrl(QUrl::fromLocalFile(str));
}
void LoaderWidnow::openDescriptorWindow()
{
    StructureDescriptor* st = new StructureDescriptor();

    if(dc->postgre || dc->customPSQL)
    {

        st->cd->setPlainText(
            "--SELECT distinct lower(table_name), lower(column_name )  -- oracle\n"
            "--FROM ALL_TAB_COLUMNS       \n"
            "--inner join   (SELECT DISTINCT  OBJECT_NAME       \n"
            "--    FROM ALL_OBJECTS           \n"
            "--    WHERE OBJECT_TYPE = 'TABLE' or OBJECT_TYPE like '%VIEW%'  AND\n"
            "--    (upper(OWNER) != 'SYSTEM' AND upper(OWNER) != 'SYS' AND upper(OWNER) != 'ADMIN' )\n"
            "--) tables on tables.OBJECT_NAME = table_name      \n"
            "--order by lower(table_name)"
            "\n"
            "\n"
            "\n"
            "SELECT table_name, column_name -- Postgres\n"
            "FROM information_schema.columns\n"
            "WHERE table_schema = 'public';    \n"
            "\n"
            "\n"
            "\n"
            "--SQLite is automatic, just press connect again\n"
            );
    }
    if(cd->textCursor().selectedText().size()>0)
        st->Init(cd->textCursor().selectedText());

    st->show();
}


//giant show() hide() walls to show/hide certan parts
void LoaderWidnow::ShowGraph()
{
    if(gw.groupBysb.isVisible())
    {
        QList<int> sizes = ui->splitter_2->sizes();
        if(sizes[1] > 0)
        {
            sizes[1] =  0;
        }
        ui->splitter_2->setSizes(sizes);
        gw.graphThemeCheckBox.hide();
        gw.showLabelsCheckBox.hide();
        gw.saveAsPDFButton.hide();
        gw.copyScriptHandle.hide();
        gw.groupBysb.hide();
        gw.separateBysb.hide();
        gw.dataColumnsb.hide();
        gw.groupByLabel.hide();
        gw.separateByLabel.hide();
        gw.dataColumnLabel.hide();
        gw.buildGraphButton.hide();
        gw.cv.hide();
        gw.ls.hide();
        gw.bs.hide();
        gw.graphTypeCB.hide();
        gw.graphTypeLabel.hide();
    }
    else
    {
        QList<int> sizes = ui->splitter_2->sizes();
        if(sizes[1] < 300)
        {
            sizes[1] =  300;
        }
        ui->splitter_2->setSizes(sizes);

        gw.graphThemeCheckBox.show();
        gw.showLabelsCheckBox.show();
        gw.saveAsPDFButton.show();
        gw.copyScriptHandle.show();
        gw.groupBysb.show();
        gw.separateBysb.show();
        gw.dataColumnsb.show();
        gw.groupByLabel.show();
        gw.separateByLabel.show();
        gw.dataColumnLabel.show();
        gw.buildGraphButton.show();
        gw.cv.show();
        gw.ls.show();
        gw.bs.show();
        gw.graphTypeCB.show();
        gw.graphTypeLabel.show();

        on_graph_group_change(gw.groupBysb.value());
        on_graph_separator_change(gw.separateBysb.value());
        on_graph_data_change(gw.dataColumnsb.value());

    }
}
void LoaderWidnow::ShowHistoryWindow()
{
    qDebug()<<"ShowHistoryWindow()";
    b_showWorkspaceWindow = false;
    b_showHistoryWindow = !b_showHistoryWindow;
    if(b_showHistoryWindow)
    {
        workspace_model->setRootPath(documentsDir + "/sqlBackup");
        workspace_tree->setModel(workspace_model);
        workspace_tree->setRootIndex(workspace_model->index(documentsDir + "/sqlBackup"));
        workspace_tree->show();
        workspace_tree->setFocus();
        QList<int> sizes = ui->CodeEditorSplitter->sizes();
        if(sizes[0] <300)
        {
            sizes[0] =  300;
        }
        ui->CodeEditorSplitter->setSizes(sizes);
    }
    if(!b_showHistoryWindow)
    {
        workspace_tree->hide();
        cd->setFocus();
        QList<int> sizes = ui->CodeEditorSplitter->sizes();
        if(sizes[0] > 0)
        {
            sizes[0] =  0;
        }
        ui->CodeEditorSplitter->setSizes(sizes);
    }
}
void LoaderWidnow::ShowWorkspacesWindow()
{
    qDebug()<<"ShowWorkspacesWindow()";
    b_showHistoryWindow = false;
    b_showWorkspaceWindow = !b_showWorkspaceWindow;
    if(b_showWorkspaceWindow)
    {

        QString prefix = documentsDir;
        if(QString(userDS.data["UserTheme"]["Workspace_Directory"]).trimmed() != "documents")
            prefix = QString(userDS.data["UserTheme"]["Workspace_Directory"]).trimmed();

        qDebug()<<prefix ;

        SaveWorkspace();

        workspace_model->setRootPath(prefix + "/workspaces");
        workspace_tree->setModel(workspace_model);
        workspace_tree->setRootIndex(workspace_model->index(prefix + "/workspaces"));
        workspace_tree->show();
        workspace_tree->setFocus();
        //ui->listWidget->setFocus();
        workspace_tree->show();
        QList<int> sizes = ui->CodeEditorSplitter->sizes();
        if(sizes[0] <300)
        {
            sizes[0] =  300;
        }
        ui->CodeEditorSplitter->setSizes(sizes);
    }
    if(!b_showWorkspaceWindow)
    {
        workspace_tree->hide();
        cd->setFocus();
        QList<int> sizes = ui->CodeEditorSplitter->sizes();
        if(sizes[0] > 0)
        {
            sizes[0] =  0;
        }
        ui->CodeEditorSplitter->setSizes(sizes);
    }
}
void LoaderWidnow::ShowTimerWindow()
{
    autolaunchTimerWindowVisible= !autolaunchTimerWindowVisible;
    if(!autolaunchTimerWindowVisible)
    {
        ui->timerdayMonthly->hide();
        ui->timerDayWeekly->hide();
        ui->timerHourWeekly->hide();
        ui->timerHourDaily->hide();
        ui->timerHourMonthly->hide();
        ui->timerMinuteDaily->hide();
        ui->timerMinuteWeekly->hide();
        ui->timerMinuteMonthly->hide();
        ui->timer_checkBox->hide();
        ui->timer_checkBox_2->hide();
        ui->timer_checkBox_3->hide();
        ui->timerMainLabel->hide();
        ui->timerRemainingTime->hide();
        ui->timerLastLaunchTime->hide();
        ui->updateGraphCheckBox->hide();
        ui->realTimeUpdatecheckBox->hide();
    }
    else
    {
        QList<int> sizes = ui->CodeEditorSplitter->sizes();
        if(sizes[0] < 100)
        {
            sizes[0] =  100;
        }
        ui->CodeEditorSplitter->setSizes(sizes);
        ui->timerdayMonthly->show();
        ui->timerDayWeekly->show();
        ui->timerHourWeekly->show();
        ui->timerHourDaily->show();
        ui->timerHourMonthly->show();
        ui->timerMinuteDaily->show();
        ui->timerMinuteWeekly->show();
        ui->timerMinuteMonthly->show();
        ui->timer_checkBox->show();
        ui->timer_checkBox_2->show();
        ui->timer_checkBox_3->show();
        ui->timerMainLabel->show();
        ui->timerRemainingTime->show();
        ui->timerLastLaunchTime->show();
        ui->updateGraphCheckBox->show();
        ui->realTimeUpdatecheckBox->show();

    }
}

// graphs
void LoaderWidnow::on_graph_group_change(int val)
{
    if(val>=0 && val <  dc->data.headers.size())
        gw.groupByLabel.setText("group by " +  dc->data.headers[val]);
    else
        gw.groupByLabel.setText("group by nothing!");
}
void LoaderWidnow::on_graph_separator_change(int val)
{
    if(val>=0 && val <  dc->data.headers.size())
        gw.separateByLabel.setText("separate by " +  dc->data.headers[val]);
    else
        gw.separateByLabel.setText("separate by nothing");
}
void LoaderWidnow::on_graph_data_change(int val)
{
    if(val>=0 && val <  dc->data.headers.size())
        gw.dataColumnLabel.setText("data column: " +  dc->data.headers[val]);
    else
        gw.dataColumnLabel.setText("no data!");
}

void LoaderWidnow::UpdateGraph()
{
    // set theme
    if(gw.graphThemeCheckBox.isChecked())
    {
        gw.cv.setBackgroundBrush(QColor::fromRgb(255,255,255));
        gw.chrt.setBackgroundBrush(QColor::fromRgb(255,255,255));
    }
    if(!gw.graphThemeCheckBox.isChecked())
    {
        gw.cv.setBackgroundBrush(QColor::fromRgb(24,24,24));
        gw.chrt.setBackgroundBrush(QColor::fromRgb(24,24,24));
    }
    int groupColumn = gw.groupBysb.value();
    int dataColumn = gw.dataColumnsb.value();
    int separateColumn = gw.separateBysb.value();

    if(gw.manual_override)
    {

        groupColumn = gw.manual_groupColumn;
        dataColumn = gw.manual_dataColumn;
        separateColumn = gw.manual_separateColumn;
    }

    bool separate = true;
    if(separateColumn <= -1 || dataColumn == separateColumn || groupColumn == separateColumn || separateColumn >= dc->data.tbldata.size())
        separate = false;

    if(dataColumn  >= dc->data.tbldata.size())
        return;
    if(groupColumn  >= dc->data.tbldata.size())
        return;

    std::map<QString,std::map<QString,float>> ColumnData; // ColumnData[separator][grouper] == value

    long int maxi = -1000000;
    long int mini = QDateTime::currentSecsSinceEpoch();
    QDateTime maxdt = QDateTime::currentDateTime();
    QDateTime mindt = QDateTime::currentDateTime();
    double maxf = -100000000.0;
    double minf = 100000000.0;
    bool bottomAxisIsDate = false;
    QStringList names;

    if(dc->data.tbldata.size() <=0)
        return;
    if(dc->data.tbldata[groupColumn].size() <1)
        return;

    // use dateAxis or valueAxis
    if(dc->data.tbldata[groupColumn].size() >=2)
    {
        bottomAxisIsDate = QVariant(dc->data.tbldata[groupColumn][0]).toDateTime().isValid() && !QVariant(dc->data.tbldata[groupColumn][0]).toDateTime().isNull();
    }
    else
        bottomAxisIsDate = false;


    // fill ColumnData
    for(int i=0;i < dc->data.tbldata[groupColumn].size();i++)
    {
        bool isReal = false;
        float a = QVariant(dc->data.tbldata[dataColumn][i]).toReal(&isReal);
        if(!isReal)
            a=1; // count
        if(separate)
            ColumnData[dc->data.tbldata[separateColumn][i]][dc->data.tbldata[groupColumn][i]] += a;
        else
            ColumnData["Value1"][dc->data.tbldata[groupColumn][i]] += a;
    }


    // get min and max
    for(auto a : ColumnData) // separation
    {
        for(auto i : a.second) // group
        {
            if(dc->data.tbldata.size() > 1)
            {

                if(i.second > maxf)
                    maxf = i.second;
                if(i.second < minf)
                    minf = i.second;

                if(!bottomAxisIsDate)
                {
                    if(mini > i.first.toInt())
                        mini = i.first.toInt();
                    if(maxi < i.first.toInt())
                        maxi = i.first.toInt();
                }
                else
                {
                    if(QVariant(i.first).toDateTime().toSecsSinceEpoch() > maxi)
                    {
                        maxi = QVariant(i.first).toDateTime().toSecsSinceEpoch();
                        maxdt = QVariant(i.first).toDateTime();
                    }
                    if(QVariant(i.first).toDateTime().toSecsSinceEpoch() < mini)
                    {
                        mini = QVariant(i.first).toDateTime().toSecsSinceEpoch();
                        mindt = QVariant(i.first).toDateTime();
                    }
                }
            }
        }
    }
    /*
    for(int i=0;i<dc->data.tbldata[dataColumn].size();i++)
    {
        if(dc->data.tbldata.size() > 1)
        {

            if(dc->data.tbldata[dataColumn][i].toFloat() > maxf)
                maxf = dc->data.tbldata[dataColumn][i].toFloat();
            if(dc->data.tbldata[dataColumn][i].toFloat() < minf)
                minf = dc->data.tbldata[dataColumn][i].toFloat();

            if(!bottomAxisIsDate)
            {
                if(mini > dc->data.tbldata[groupColumn][i].toInt())
                    mini = dc->data.tbldata[groupColumn][i].toInt();
                if(maxi < dc->data.tbldata[groupColumn][i].toInt())
                    maxi = dc->data.tbldata[groupColumn][i].toInt();
            }
            else
            {
                if(dc->data.tbldata[groupColumn][i].toDateTime().toSecsSinceEpoch() > maxi)
                {
                    maxi = dc->data.tbldata[groupColumn][i].toDateTime().toSecsSinceEpoch();
                    maxdt = dc->data.tbldata[groupColumn][i].toDateTime();
                }
                if(dc->data.tbldata[groupColumn][i].toDateTime().toSecsSinceEpoch() < mini)
                {
                    mini = dc->data.tbldata[groupColumn][i].toDateTime().toSecsSinceEpoch();
                    mindt = dc->data.tbldata[groupColumn][i].toDateTime();
                }
            }
        }

    }
    */



    qDebug()<<mindt ;
    qDebug()<<maxdt ;

    qDebug()<<minf ;
    qDebug()<<maxf ;
    if(minf > 0.0f)
        minf=  0.0f;

    maxf *= 1.0f;

    // graph type setups
    if(gw.graphTypeCB.currentText() == "Spline")
    {
        for(int i=0;i< ColumnData.size();i++)
        {
            QSplineSeries* ls = new QSplineSeries();
            gw.line_series.push_back(ls);
        }
    }
    else if (gw.graphTypeCB.currentText() == "Line")
    {
        for(int i=0;i< ColumnData.size();i++)
        {
            QLineSeries* ls = new QLineSeries();
            gw.Straight_line_series.push_back(ls);
        }
    }

    gw.ls.clear();
    //gw.chrt.removeAllSeries(); // crashes the app, doing it by hand
    gw.chrt.removeSeries(&gw.ls);
    gw.vay.setGridLineColor(QColor::fromRgbF(0,0,0,0));
    gw.da.setGridLineColor(QColor::fromRgbF(0,0,0,0));

    for(int i=0;i<gw.line_series.size();i++)
    {
        gw.line_series[i]->clear();
        gw.chrt.removeSeries(gw.line_series[i]);

        //gw.line_series[i]->setPointLabelsVisible();
        //gw.line_series[i]->setPointLabelsColor(gw.line_series[i]->pen().color());
        //qDebug()<<gw.line_series[i]->pen().color();
        //gw.line_series[i]->setPointLabelsFormat("@yPoint");
        //gw.line_series[i]->setPointLabelsClipping(false);
    }

    for(int i=0;i<gw.Straight_line_series.size();i++)
    {
        gw.Straight_line_series[i]->clear();
        gw.chrt.removeSeries(gw.Straight_line_series[i]);

        //gw.Straight_line_series[i]->setPointLabelsVisible();
        //gw.Straight_line_series[i]->setPointLabelsColor(gw.Straight_line_series[i]->pen().color());
        //qDebug()<<gw.Straight_line_series[i]->pen().color();
        //gw.Straight_line_series[i]->setPointLabelsFormat("@yPoint");
        //gw.Straight_line_series[i]->setPointLabelsClipping(false);
    }

    gw.bs.clear();
    gw.chrt.removeSeries(&gw.bs);

    int ls_iter = 0;
    gw.bar_sets.clear();
    // fill series with data
    for(auto x : ColumnData) // each separator
    {
        if(gw.graphTypeCB.currentText() == "Spline" || gw.graphTypeCB.currentText() == "Line")
        {
            QLineSeries* ls = nullptr;
            if(gw.graphTypeCB.currentText() == "Spline")
                ls = gw.line_series[ls_iter];
            if(gw.graphTypeCB.currentText() == "Line")
                ls = gw.Straight_line_series[ls_iter];


            ls->setName(x.first);
            for(auto i : x.second) // each group
            {
                qDebug()<<(QVariant(i.first).toDateTime().toSecsSinceEpoch()-mini)/double(maxi - mini) * 1.0f << i.second;
                if(!bottomAxisIsDate && QVariant(i.first).toInt() > 0)
                    ls->append((QVariant(i.first).toInt() - mini)/float(maxi-mini) * 1.0f,i.second);
                else if(bottomAxisIsDate)
                    ls->append((QVariant(i.first).toDateTime().toSecsSinceEpoch()-mini)/double(maxi - mini) * 1.0f,i.second);
            }
        }
        else if(gw.graphTypeCB.currentText() == "Bar")
        {
            QBarSet* set = new QBarSet(x.first);
            gw.bar_sets.push_back(set);


            set->remove(0,set->count());
            set->setLabel(x.first);
            for(auto i : x.second) // each group
            {
                set->append(i.second);
            }
            gw.bs.append(set);
        }


        ls_iter++;
    }



    gw.vay.setMax(maxf);
    gw.vay.setMin(minf);

    if(gw.graphTypeCB.currentText() == "Spline")
    {
        for(int i=0;i< ColumnData.size();i++)
            gw.chrt.addSeries(gw.line_series[i]);
    }
    else if (gw.graphTypeCB.currentText() == "Line")
    {
        for(int i=0;i< ColumnData.size();i++)
            gw.chrt.addSeries(gw.Straight_line_series[i]);
    }
    else if (gw.graphTypeCB.currentText() == "Bar")
    {
        gw.chrt.addSeries(&gw.bs);
    }

    // setup chart
    if(!bottomAxisIsDate)
    {
        gw.chrt.removeAxis(&gw.da);
        gw.chrt.addAxis(&gw.vax,Qt::Alignment::enum_type::AlignBottom);
        gw.vax.setMax(maxi);
        gw.vax.setMin(0);
    }
    else
    {
        gw.chrt.removeAxis(&gw.vax);
        gw.chrt.addAxis(&gw.da,Qt::Alignment::enum_type::AlignBottom);
        gw.da.setMax(maxdt);
        gw.da.setMin(mindt);
    }

    ls_iter = 0;
    for(int i = 0 ;i < gw.line_series.size();i++) // each separator
    {

        gw.line_series[i]->attachAxis(&gw.vay);
        gw.vay.setMax(maxf);
        gw.vay.setMin(minf);
        if(gw.showLabelsCheckBox.isChecked())
        {
            gw.line_series[i]->setPointLabelsColor(gw.line_series[i]->pen().color());
            qDebug()<<gw.line_series[i]->pen().color();
            gw.line_series[i]->setPointLabelsFormat("@yPoint");
            gw.line_series[i]->setPointLabelsClipping(false);


            gw.line_series[i]->setPointLabelsColor(gw.line_series[i]->pen().color());
            gw.line_series[i]->setPointLabelsFormat("@yPoint");
            gw.line_series[i]->setPointLabelsClipping(false);
            QFont fnt =  gw.line_series[i]->pointLabelsFont();
            fnt.setBold(true);
            gw.line_series[i]->setPointLabelsFont(fnt);
        }
        gw.line_series[i]->setPointsVisible();



        gw.line_series[i]->setPointLabelsVisible(gw.showLabelsCheckBox.isChecked());
    }



    // series formating setup, after they were added to chart
    for(int i = 0 ;i < gw.Straight_line_series.size();i++) // each separator
    {
        gw.Straight_line_series[i]->attachAxis(&gw.vay);
        gw.vay.setMax(maxf*1.05f);
        gw.vay.setMin(minf);
        if(gw.showLabelsCheckBox.isChecked())
        {
            gw.Straight_line_series[i]->setPointLabelsColor(gw.Straight_line_series[i]->pen().color());
            qDebug()<<gw.Straight_line_series[i]->pen().color();
            gw.Straight_line_series[i]->setPointLabelsFormat("@yPoint");
            gw.Straight_line_series[i]->setPointLabelsClipping(false);

            gw.Straight_line_series[i]->setPointLabelsColor(gw.Straight_line_series[i]->pen().color());
            gw.Straight_line_series[i]->setPointLabelsFormat("@yPoint");
            gw.Straight_line_series[i]->setPointLabelsClipping(false);
            QFont fnt =  gw.Straight_line_series[i]->pointLabelsFont();
            fnt.setBold(true);
            gw.Straight_line_series[i]->setPointLabelsFont(fnt);
        }
        gw.Straight_line_series[i]->setPointLabelsVisible(gw.showLabelsCheckBox.isChecked());
    }

    for(int i = 0 ;i < gw.bar_sets.size();i++) // each separator
    {
        gw.bar_sets[i]->setBorderColor(gw.bar_sets[i]->brush().color());
        gw.bar_sets[i]->pen().setWidthF(0);
        gw.bar_sets[i]->setLabelColor(gw.bar_sets[i]->brush().color());

        gw.bar_sets[i]->setBorderColor(gw.bar_sets[i]->pen().color());
        //set->setLabelColor(QColor::fromRgb(0,0,0));
        QFont fnt = gw.bar_sets[i]->labelFont();
        fnt.setBold(true);
        gw.bar_sets[i]->setLabelFont(fnt);


    }

    if(gw.showLabelsCheckBox.isChecked())
    {
        gw.bs.setLabelsPosition(QAbstractBarSeries::LabelsOutsideEnd);
        gw.bs.setLabelsVisible(true);
    }
    else
        gw.bs.setLabelsVisible(false);

    gw.bs.attachAxis(&gw.vay);

    gw.vay.setMax(maxf*1.10f);
    gw.vay.setMin(minf);

}
void LoaderWidnow::saveGraphAsPDF()
{




    QImage image(1980, 1020, QImage::Format_ARGB32);
    image.fill(Qt::white);
    QPainter painter(&image);

    QSize sz = gw.cv.size();
    QSize max_sz = gw.cv.maximumSize();
    QSize min_sz = gw.cv.minimumSize();

    gw.cv.setMaximumSize({1980, 1020});
    gw.cv.setMinimumSize({1980, 1020});
    gw.cv.show();
    gw.cv.render(&painter);
    gw.cv.setMaximumSize(max_sz);
    gw.cv.setMinimumSize(min_sz);



    painter.end();
    image.save(documentsDir + "/" + gw.savename);

}
void LoaderWidnow::copyGraphScriptHandle()
{
    QString selected_text = "";

    // create a ready-to-paste script to save current graph
    selected_text = "SubexecToGraph { {";
    selected_text += dc->driver;
    selected_text += "} {";
    selected_text += dc->dbname;
    selected_text += "} {";
    selected_text += "FILE_NAME";
    selected_text += "} {";
    selected_text += QVariant(gw.groupBysb.value()).toString();
    selected_text += "} {";
    selected_text += QVariant(gw.separateBysb.value()).toString();
    selected_text += "} {";
    selected_text += QVariant(gw.dataColumnsb.value()).toString();
    selected_text += "}";



    selected_text += "\n\t";
    QString str = dc->sqlCode;
    selected_text += str.replace('\n', "\n\t");
    while (selected_text.endsWith('\t'))
        selected_text.remove(selected_text.size()-1,1);
    selected_text += "}";
    QApplication::clipboard()->setText(selected_text);
}
void LoaderWidnow::saveGraph(DatabaseConnection* tmp_con)
{

    gw.groupBysb.setValue(gw.manual_groupColumn);
    gw.dataColumnsb.setValue(gw.manual_dataColumn);
    gw.separateBysb.setValue(gw.manual_separateColumn);

    DatabaseConnection* tmp_dc = dc;
    dc = tmp_con;
    UpdateGraph();
    saveGraphAsPDF();
    dc = tmp_dc;
    gw.manual_override= false;
}
// Table widget copying
void LoaderWidnow::CopySelectionFormTable()
{
    qDebug()<<"CopySelectionFormTable()";
    if(ui->tableWidget->selectedItems().size()<=0)
        return;
    QModelIndexList indexes = ui->tableWidget->selectionModel()->selectedIndexes();
    QAbstractItemModel * model = ui->tableWidget->model();
    QModelIndex previous = indexes.first();
    indexes.removeFirst();
    QString selected_text =  model->data(previous).toString();
    bool first = true;
    for(auto current : indexes)
    {
        if (current.row() != previous.row())
        {
            selected_text.append('\n');
        }
        else
        {
            selected_text.append('\t');
        }
        QVariant data = model->data(current);
        QString text = data.toString();
        selected_text.append(text);

        previous = current;
    }
    QApplication::clipboard()->setText(selected_text);
}
void LoaderWidnow::CopySelectionFormTableSql()
{
    qDebug()<<"CopySelectionFormTableSql()";
    if(ui->tableWidget->selectedItems().size()<=0)
        return;
    QModelIndexList indexes = ui->tableWidget->selectionModel()->selectedIndexes();
    QAbstractItemModel * model = ui->tableWidget->model();
    int column_count = model->columnCount();
    QModelIndex previous = indexes.first();
    indexes.removeFirst();
    QString selected_text =  "";

    selected_text.append(" '");
    selected_text.append(model->data(previous).toString());
    selected_text.append("', \n");
    bool first = true;
    qDebug()<<"ctr+shift+c'ed";
    qDebug()<<"column_count " << column_count;

    for(auto current : indexes)
    {
        QVariant data = model->data(current);
        QString text = data.toString();
        selected_text.append(" '");
        selected_text.append(text);
        selected_text.append("', \n");

        previous = current;
    }
    QApplication::clipboard()->setText(selected_text);
    return;
}

// copy last unrolled sql
void LoaderWidnow::CopyLastLaunchedSql()
{
    QApplication::clipboard()->setText(dc->Last_sqlCode);

}

// code edtitor
void LoaderWidnow::updatesuggestion()
{
    //ui->suggestionLabel->setText(cd->lastSuggestedWord);
    ui->textPosLabel->setText(QVariant(cd->textCursor().blockNumber() + 1).toString() + ":" + QVariant(cd->textCursor().positionInBlock() + 1).toString() + "(" + QVariant(cd->textCursor().position() + 1).toString() + ")");
}
void LoaderWidnow::FillSuggest()
{
    qDebug()<<"FillSuggest()";
    cd->FillsuggestName();

}
void LoaderWidnow::replaceTool()
{
    qDebug() <<"replaceTool()";
    ReplaceWindow* rd = new ReplaceWindow();


    QTextCursor cursor = cd->textCursor();
    cursor.setPosition(cd->textCursor().selectionStart());
    rd->start_line = cursor.blockNumber();


    cursor.setPosition(cd->textCursor().selectionEnd());
    rd->end_line = cursor.blockNumber();

    rd->replaceWhat = "";
    if( rd->start_line != rd->end_line)
    {
        QTextCursor cursor = cd->textCursor();
        cursor.setPosition(cd->textCursor().selectionStart());
        rd->start_line = cursor.blockNumber()+1;


        cursor.setPosition(cd->textCursor().selectionEnd());
        rd->end_line = cursor.blockNumber()+1;
    }
    else if(cd->textCursor().selectedText().size()>0)
    {
        rd->replaceWhat = cd->textCursor().selectedText();
        rd->start_line = _replacePrev_from;
        rd->end_line = _replacePrev_to;


    }
    else
    {
        rd->replaceWhat = _replacePrev_What;
        rd->start_line = _replacePrev_from;
        rd->end_line = _replacePrev_to;

    }
    rd->replaceWith = _replacePrev_With;
    rd->Init();
    rd->show();
    // if(rd.exec())
    // {// do the raplacing
    //     cd->replace(rd.start_line,rd.end_line,rd.replaceWhat,rd.replaceWith);
    //     qDebug() <<"replaced";
    // }
    // _replacePrev_What = rd.replaceWhat;
    // _replacePrev_With = rd.replaceWith;
    // _replacePrev_from = rd.start_line;
    // _replacePrev_to = rd.end_line;
}

void LoaderWidnow::replaceAction()
{

    if(replaceWindow != nullptr)
    {// do the raplacing
        cd->replace(replaceWindow->start_line,replaceWindow->end_line,replaceWindow->replaceWhat,replaceWindow->replaceWith);
        qDebug() <<"replaced";
    }
    _replacePrev_What = replaceWindow->replaceWhat;
    _replacePrev_With = replaceWindow->replaceWith;
    _replacePrev_from = replaceWindow->start_line;
    _replacePrev_to = replaceWindow->end_line;
    replaceWindow = nullptr;
}
void LoaderWidnow::findAction()
{

    if(replaceWindow != nullptr)
        cd->FindNext(replaceWindow->replaceWhat);

    replaceWindow = nullptr;
}

void LoaderWidnow::CommentSelected()
{
    cd->CommentSelected();
}

void _async_save_to_xlsx(LoaderWidnow* ldw, QString savetext)
{
    qDebug()<<"on_SaveXLSXButton_pressed()";
    QString str = documentsDir + "/excel/";

    str += userDS.data["ExcelPrefixAliases"][ldw->cd->highlighter->dbSchemaName];
    str += savetext;
    if(str.size() > 200)// backup against too long filenames, cuz windosheet
        str.resize(200);
    if(!str.endsWith(".xlsx"))
        str += ".xlsx";
    ldw->dc->data.sqlCode = ldw->dc->sqlCode;
    ldw->dc->data.allSqlCode = ldw->cd->toPlainText();

    ldw->dc->data.lastExportSuccess = ldw->dc->data.ExportToExcel(str,0,0,0,0,true);

    // if(dc->data.lastExportSuccess)
    //     ui->miscStatusLabel->setText(QString("Saved as XLSX ") + str);
    // else
    //     ui->miscStatusLabel->setText(QString("Failed to save xlsx, file probably opened") + str);

    ldw->executionTimer.stop();
}

void _async_save_to_csv(LoaderWidnow* ldw, QString savetext)
{
    qDebug()<<"on_SaveCSVButton_pressed()";
    QString str = documentsDir + "/CSV/";
    if(!ldw->cd->highlighter->dbSchemaName.contains('.'))
        str += ldw->cd->highlighter->dbSchemaName;
    str += savetext;
    if(str.size() > 200)// backup against too long filenames, cuz windosheet
        str.resize(200);
    if(!str.endsWith(".csv"))
        str += ".csv";

    ldw->dc->data.lastExportSuccess = ldw->dc->data.ExportToCSV(str,';',true);
    // if(dc->data.lastExportSuccess)
    //     ui->miscStatusLabel->setText(QString("Saved as CSV ") + str);
    // else
    //     ui->miscStatusLabel->setText(QString("failed to save to CSV, file probably opened") + str);

    ldw->executionTimer.stop();
}

void _async_save_to_local(LoaderWidnow* ldw, QString savetext)
{
    qDebug()<<"on_SaveCSVButton_pressed()";
    ldw->dc->data.lastExportSuccess = ldw->dc->data.ExportToSQLiteTable(savetext);


    // if(dc->data.lastExportSuccess)
    //     ui->miscStatusLabel->setText(QString("Saved to local database, table:") + str);
    // else
    //     ui->miscStatusLabel->setText(QString("Failed to save to local database, table: ") + str);

    ldw->executionTimer.stop();
}
// export
void LoaderWidnow::on_SaveXLSXButton_pressed()
{
    qDebug()<<"on_SaveXLSXButton_pressed()";

    if(((*sqlexecThread)!= nullptr && (*sqlexecThread)->isRunning()) || dc->data.exporting || dc->executing)
    {
        return;
    }
    ui->stopLoadingQueryButton->show();
    ui->pushButton_3->hide();
    ui->miscStatusLabel->setText("running sql...");
    (*sqlexecThread) = QThread::create(_async_save_to_xlsx,this,ui->saveLineEdit->text());
    (*sqlexecThread)->start();
    ui->miscStatusLabel->setText("running sql...");

    executionTimer.setSingleShot(false);
    executionTimer.setTimerType(Qt::CoarseTimer);
    executionTimer.setInterval(15);
    executionTimer.start();
}
void LoaderWidnow::on_SaveCSVButton_pressed()
{
    qDebug()<<"on_SaveCSVButton_pressed()";

    if(((*sqlexecThread)!= nullptr && (*sqlexecThread)->isRunning()) || dc->data.exporting || dc->executing)
    {
        return;
    }
    ui->stopLoadingQueryButton->show();
    ui->pushButton_3->hide();

    ui->miscStatusLabel->setText("running sql...");
    (*sqlexecThread) = QThread::create(_async_save_to_csv,this,ui->saveLineEdit->text());
    (*sqlexecThread)->start();
    ui->miscStatusLabel->setText("running sql...");

    executionTimer.setSingleShot(false);
    executionTimer.setTimerType(Qt::CoarseTimer);
    executionTimer.setInterval(15);
    executionTimer.start();

}
void LoaderWidnow::on_SaveSQLiteButton_pressed()
{
    qDebug()<<"on_SaveSQLiteButton_pressed()";

    if(((*sqlexecThread)!= nullptr && (*sqlexecThread)->isRunning()) || dc->data.exporting || dc->executing)
    {
        return;
    }
    ui->stopLoadingQueryButton->show();
    ui->pushButton_3->hide();
    ui->miscStatusLabel->setText("running sql...");
    (*sqlexecThread) = QThread::create(_async_save_to_local,this,ui->saveLineEdit->text());
    (*sqlexecThread)->start();
    ui->miscStatusLabel->setText("running sql...");

    executionTimer.setSingleShot(false);
    executionTimer.setTimerType(Qt::CoarseTimer);
    executionTimer.setInterval(15);
    executionTimer.start();

}

void LoaderWidnow::on_exportDone()
{
    executionTimer.stop();

    if(dc->data.saveRowSize >0)
        ui->exportProgressBar->setMaximum(dc->data.saveRowSize);
    else
        ui->exportProgressBar->setMaximum(1);
    ui->exportProgressBar->setValue(dc->data.saveRowsDone);

    if(!dc->executing)
    {
        ui->stopLoadingQueryButton->hide();
        ui->pushButton_3->show();
    }


    if (dc->data.lastexporttype == "csv")
    {
        if(dc->data.lastExportSuccess)
            ui->miscStatusLabel->setText(QString("Saved as CSV ") + ui->saveLineEdit->text()
                                             + " exporttime: " + dc->data.LastSaveDuration.toString() + "." +  QVariant(dc->data.LastSaveDuration.msec()).toString());
        else
            ui->miscStatusLabel->setText(QString("failed to save to CSV, file probably opened") + ui->saveLineEdit->text()
                                             + " exporttime: " + dc->data.LastSaveDuration.toString() + "." +  QVariant(dc->data.LastSaveDuration.msec()).toString());
    }
    else if (dc->data.lastexporttype == "xlsx")
    {
        if(dc->data.lastExportSuccess)
            ui->miscStatusLabel->setText(QString("Saved as XLSX ") + ui->saveLineEdit->text()
                                             + " exporttime: " + dc->data.LastSaveDuration.toString() + "." +  QVariant(dc->data.LastSaveDuration.msec()).toString());
        else
            ui->miscStatusLabel->setText(QString("Failed to save xlsx, file probably opened") + ui->saveLineEdit->text()
                                             + " exporttime: " + dc->data.LastSaveDuration.toString() + "." +  QVariant(dc->data.LastSaveDuration.msec()).toString());
    }
    else if (dc->data.lastexporttype == "Local")
    {
        if(dc->data.lastExportSuccess)
            ui->miscStatusLabel->setText(QString("Saved to local database, table:") + ui->saveLineEdit->text()
                                             + " exporttime: " + dc->data.LastSaveDuration.toString() + "." +  QVariant(dc->data.LastSaveDuration.msec()).toString());
        else
            ui->miscStatusLabel->setText(QString("Failed to save to local database, table: ") + ui->saveLineEdit->text()
                                             + " exporttime: " + dc->data.LastSaveDuration.toString() + "." +  QVariant(dc->data.LastSaveDuration.msec()).toString());
    }
}

void LoaderWidnow::on_ImportFromCSVButton_pressed()
{

    dc->data.silentExcelImport=false;
    dc->data.ImportFromCSV(QFileDialog::getOpenFileName(this, tr("Select csv file")),';',true);
    UpdateTable();
    ui->tableDBNameLabel->setText("Imported from csv");
    dc->data.silentExcelImport=true;
}
void LoaderWidnow::on_importFromExcelButton_pressed()
{
    dc->data.silentExcelImport=false;
    dc->data.ImportFromExcel(QFileDialog::getOpenFileName(),0,0,0,0,true);
    UpdateTable();
    ui->tableDBNameLabel->setText("Imported from Excel");
    dc->data.silentExcelImport=true;
}

//Workspaces, history
void LoaderWidnow::SaveWorkspace()
{
    qDebug()<<"SaveWorkspace()";

    QString prefix = documentsDir;
    if(QString(userDS.data["UserTheme"]["Workspace_Directory"]).trimmed() != "documents")
        prefix = QString(userDS.data["UserTheme"]["Workspace_Directory"]).trimmed();

    if(LastWorkspaceName.contains(":"))
    {// saving into global space
        std::ofstream stream2 ((LastWorkspaceName).toLocal8Bit());
        QString text = cd->toPlainText();
        stream2.clear();
        QStringList tokens;

        if(text.startsWith("-- {"))
        {
            QString text2 = text;
            text.clear();
            text.reserve(text2.size());
            int newlinecounter = 0;
            for(int i=0;i<text2.size();i++)
            {
                if(newlinecounter < 1)
                {
                    if(text2[i] == '\n')
                        newlinecounter += 1;
                }
                else
                {
                    text.push_back(text2[i]);

                }
            }
        }
        stream2 << "-- {" << ui->driverComboBox->currentText().toStdString() << "} ";
        stream2 << " {" <<   ui->DBNameComboBox->currentText().toStdString() << "}\n";
        stream2 << text.toStdString();
        stream2.close();
    }
    else if(!LastWorkspaceName.endsWith(".sql"))
        LastWorkspaceName+=".sql";
    std::ofstream stream2 ((QString(prefix + "/workspaces/") + LastWorkspaceName).toLocal8Bit());
    QString text = cd->toPlainText();
    stream2.clear();
    QStringList tokens;

    if(text.startsWith("-- {"))
    {
        QString text2 = text;
        text.clear();
        text.reserve(text2.size());
        int newlinecounter = 0;
        for(int i=0;i<text2.size();i++)
        {
            if(newlinecounter < 1)
            {
                if(text2[i] == '\n')
                    newlinecounter += 1;
            }
            else
            {
                text.push_back(text2[i]);

            }
        }
    }
    stream2 << "-- {" << ui->driverComboBox->currentText().toStdString() << "} ";
    stream2 << " {" <<   ui->DBNameComboBox->currentText().toStdString() << "}\n";
    stream2 << text.toStdString();
    stream2.close();

    this->setWindowTitle(LastWorkspaceName);
}
void LoaderWidnow::on_workspaceLineEdit_textChanged(QString arg1)
{
    LastWorkspaceName = arg1;
    this->setWindowTitle(LastWorkspaceName);
}
void LoaderWidnow::on_TreeItem_Changed(const QItemSelection &selected, const QItemSelection &deselected)
{
    qDebug() << "selection changed";

    if(selected == deselected || selected.empty())
    {

        qDebug() << "but selection was not changed";
        return;
    }

    QString currentText = workspace_model->filePath(selected.back().indexes().back());

    QString prefix = documentsDir;
    if(QString(userDS.data["UserTheme"]["Workspace_Directory"]).trimmed() != "documents")
        prefix = QString(userDS.data["UserTheme"]["Workspace_Directory"]).trimmed();

    currentText = currentText.mid(workspace_model->rootDirectory().path().size() +1);


    qDebug() << "currentText " << currentText ;
    if(b_showWorkspaceWindow && currentText =="")
        return;

    if(currentText == "tmp")
        cd->setPlainText(tmpSql);
    else
    {
        QString filename = currentText;
        if(b_showWorkspaceWindow)
        {


            filename = prefix + "/workspaces/" + currentText;

            LastWorkspaceName = currentText;
            ui->tabWidget->setTabText(ui->tabWidget->currentIndex(),currentText);
            on_workspaceLineEdit_textChanged(currentText);
        }
        else
        {
            filename = documentsDir +"/sqlBackup/"+filename;
            LastWorkspaceName= "tmp_history";
            ui->tabWidget->setTabText(ui->tabWidget->currentIndex(),"tmp_history");
            on_workspaceLineEdit_textChanged("tmp_history");

        }
        QFile file( filename);
        if (file.open(QFile::ReadOnly | QFile::Text))
            cd->setPlainText(file.readAll());
        if(b_showWorkspaceWindow)
        {
            QString text = cd->toPlainText();
            if(text.startsWith("-- {"))
            {
                QStringList tokens;
                int i = 0;
                bool inbrakets = false;
                while(i < text.size())
                {
                    if(text[i] == '{')
                    {
                        tokens.push_back("");
                        inbrakets = true;
                        i++;
                        continue;
                    }
                    if(text[i] == '}')
                    {
                        tokens.back() = tokens.back().trimmed();
                        inbrakets = true;
                        if(tokens.size() == 2)
                            break;
                        i++;
                        continue;
                    }
                    if(inbrakets)
                    {
                        tokens.back().push_back(text[i]);
                    }
                    i++;

                }
                if(tokens.size() >= 2)
                {
                    ui->driverComboBox->setCurrentText(tokens[0].trimmed());
                    ui->DBNameComboBox->setCurrentText(tokens[1].trimmed());


                    dc->driver = tokens[0].trimmed();
                    dc->dbname = tokens[1].trimmed();

                    if(userDS.Load((documentsDir + "/userdata.txt")))
                    {

                        dc->usrname = userDS.data[ui->DBNameComboBox->currentText()]["name"];
                        dc->password = userDS.data[ui->DBNameComboBox->currentText()]["password"];

                        ui->userNameLineEdit->setText(dc->usrname.trimmed());
                        ui->passwordLineEdit->setText(dc->password.trimmed());
                        dc->usrname = dc->usrname.trimmed();
                        dc->password = dc->password.trimmed();
                    }
                    else
                    {
                        ui->userNameLineEdit->setText("");
                        ui->passwordLineEdit->setText("");
                        dc->usrname =  "";
                        dc->password = "";

                    }
                }
            }
        }
    }
}
void LoaderWidnow::on_DBNameComboBox_currentTextChanged(const QString &arg1)
{
    ui->userNameLineEdit->setText(userDS.GetProperty(ui->DBNameComboBox->currentText(),"name"));
    ui->passwordLineEdit->setText(userDS.GetProperty(ui->DBNameComboBox->currentText(),"password"));
    ui->driverComboBox->setCurrentText(userDS.GetProperty(ui->DBNameComboBox->currentText(),"lastDriver"));

}

//Pause execution
void LoaderWidnow::on_stopLoadingQueryButton_pressed()
{
    if(dc!= nullptr)
        dc->stopRunning();
}
void LoaderWidnow::on_the500LinesCheckBox_checkStateChanged(const Qt::CheckState &arg1)
{
    if(arg1 == Qt::Checked)
    {
        dc->stopAt500Lines = true;
    }
    else
        dc->stopAt500Lines = false;
}


// Tabs
inline int _addtabcounter = 1;
void LoaderWidnow::cycleTabs()
{
    int indx = ui->tabWidget->currentIndex()+1;

    if(indx >= ui->tabWidget->tabBar()->count())
        indx = 0;

    ui->tabWidget->setCurrentIndex(indx);
}
void LoaderWidnow::on_tabWidget_tabCloseRequested(int index)
{
    //ui->tabWidget->removeTab(index);
    if(currentTabId == QVariant(ui->tabWidget->tabWhatsThis(index)).toInt())// its current tab, dont want to do stuff
    {
        if(ui->tabWidget->tabBar()->count() > 1)
        {
            if(index != 0)
            {
                ui->tabWidget->setCurrentIndex(0);
            }
            else
            {
                ui->tabWidget->setCurrentIndex(1);
            }
        }
        else if(_tab_failsave)return;
    }

    int delTabId = QVariant(ui->tabWidget->tabWhatsThis(index)).toInt();

    bool found = false;
    int i=0;
    for(i=0;i<tabDatas.size();i++)
    {
        if(tabDatas[i]->Id == QVariant(delTabId).toString())
        {
            found = true;
            break;
        }
    }
    if(!found)
        return;

    if(tabDatas[i]->dc.executing || tabDatas[i]->dc.dataDownloading)
    {
        tabDatas[i]->dc.stopRunning();
        ui->tabWidget->setCurrentIndex(index);
        //on_tabWidget_currentChanged(1);
    }
    else
    {

        // save sqlBacup
        QString str = "sqlBackup/";
        QDate dt = QDate::currentDate();
        str += QVariant(dt.year()).toString();
        str += "_";
        if(QVariant(dt.month() ).toString().size() <=1)
            str+="0";
        str += QVariant(dt.month()).toString();
        str += "_";
        if(QVariant(dt.day() ).toString().size() <=1)
            str+="0";
        str += QVariant(dt.day() ).toString();
        str += "_";
        str += QTime::currentTime().toString();
        str += " closed tab " + ui->tabWidget->tabText(index);
        str +=".sql";
        str.replace(":","_");
        qDebug()<<documentsDir + "/" +str;
        std::ofstream stream ((documentsDir + "/" +str).toStdString());
        stream << tabDatas[i]->cd->toPlainText().toStdString();
        stream.close();

        // just in case
        tabDatas[i]->dc.stopRunning();

        ui->edit_layout->removeWidget(tabDatas[i]->cd);
        delete tabDatas[i]->cd;
        delete tabDatas[i];
        tabDatas[i] = tabDatas.back();
        tabDatas.pop_back();

        ui->tabWidget->removeTab(index);
    }
}
void LoaderWidnow::on_tabWidget_currentChanged(int index)
{
    bool found = false;
    int i=0;
    for(i=0;i<tabDatas.size();i++)
    {
        if(tabDatas[i]->Id == QVariant(currentTabId).toString())
        {
            found = true;
            break;
        }
    }
    if(found)
    {
        tabDatas[i]->sql = cd->toPlainText();
        tabDatas[i]->workspaceName = LastWorkspaceName;

        tabDatas[i]->lastQueryState = queryExecutionState;
        tabDatas[i]->lastMessage = ui->miscStatusLabel->text();
        tabDatas[i]->lastTableMessage = ui->dataSizeLabel_2->text();
        tabDatas[i]->lastTableDBName = ui->tableDBNameLabel->text();
        tabDatas[i]->textposition = cd->textCursor().position();
        tabDatas[i]->cd->hide();
        //codeeditor
        disconnect( cd, SIGNAL(s_suggestedName()), this, SLOT(updatesuggestion()));

    }


    currentTabId = QVariant(ui->tabWidget->tabWhatsThis(index)).toInt();

    found = false;
    i=0;
    for(i=0;i<tabDatas.size();i++)
    {
        if(tabDatas[i]->Id == QVariant(currentTabId).toString())
        {
            found = true;
            break;
        }
    }
    if(!found)
        return;
    disconnect( dc, SIGNAL(queryBeginCreating()), this, SLOT(onQueryBeginCreating()));
    disconnect( dc, SIGNAL(queryBeginExecuting()), this, SLOT(onQueryBegin()));
    disconnect( dc, SIGNAL(querySuccess()), this, SLOT(onQuerySuccess()));
    disconnect( dc, SIGNAL(execedSql()), this, SLOT(UpdateTable()));

    disconnect( &dc->data, SIGNAL(ExportedToCSV()), this, SLOT(on_exportDone()));
    disconnect( &dc->data, SIGNAL(ExportedToExcel()), this, SLOT(on_exportDone()));
    disconnect( &dc->data, SIGNAL(ExportedToSQLiteTable()), this, SLOT(on_exportDone()));



    disconnect( dc, SIGNAL(sendMail(QString, QString, QString, QStringList,QStringList, QString, QString, QStringList, QStringList)), this, SLOT(sendMail(QString, QString, QString, QStringList,QStringList, QString, QString, QStringList, QStringList)) );
    disconnect( dc, SIGNAL(saveGraph(DatabaseConnection*)), this, SLOT(saveGraph(DatabaseConnection*)) );


    dc = &tabDatas[i]->dc;



    connect( dc, SIGNAL(queryBeginCreating()), this, SLOT(onQueryBeginCreating()), Qt::QueuedConnection );
    connect( dc, SIGNAL(queryBeginExecuting()), this, SLOT(onQueryBegin()), Qt::QueuedConnection );
    connect( dc, SIGNAL(querySuccess()), this, SLOT(onQuerySuccess()), Qt::QueuedConnection );
    connect( dc, SIGNAL(execedSql()), this, SLOT(UpdateTable()), Qt::QueuedConnection );

    connect( &dc->data, SIGNAL(ExportedToCSV()), this, SLOT(on_exportDone()), Qt::QueuedConnection );
    connect( &dc->data, SIGNAL(ExportedToExcel()), this, SLOT(on_exportDone()), Qt::QueuedConnection );
    connect( &dc->data, SIGNAL(ExportedToSQLiteTable()), this, SLOT(on_exportDone()), Qt::QueuedConnection );



    connect( dc, SIGNAL(sendMail(QString, QString, QString, QStringList,QStringList, QString, QString, QStringList, QStringList)), this, SLOT(sendMail(QString, QString, QString, QStringList,QStringList, QString, QString, QStringList, QStringList)), Qt::QueuedConnection );
    connect( dc, SIGNAL(saveGraph(DatabaseConnection*)), this, SLOT(saveGraph(DatabaseConnection*)), Qt::BlockingQueuedConnection );

    sqlexecThread = &tabDatas[i]->sqlexecThread;

    tabDatas[i]->cd->show();
    cd = tabDatas[i]->cd;
    cd->setFocus();
    //codeeditor
    connect( cd, SIGNAL(s_suggestedName()), this, SLOT(updatesuggestion()), Qt::QueuedConnection );
    LastWorkspaceName = tabDatas[i]->workspaceName;

    ui->tabWidget->setTabText(ui->tabWidget->currentIndex(),tabDatas[i]->workspaceName);
    on_workspaceLineEdit_textChanged(ui->tabWidget->tabText(ui->tabWidget->currentIndex()));

    ui->driverComboBox->setCurrentText(dc->driver);
    ui->DBNameComboBox->setCurrentText(dc->dbname);
    ui->userNameLineEdit->setText(dc->usrname);
    ui->passwordLineEdit->setText(dc->password);
    ui->connectionStatusLabel_2->setText(QString("connected to ") + dc->dbname);

    queryExecutionState = tabDatas[i]->lastQueryState;
    ui->miscStatusLabel->setText(tabDatas[i]->lastMessage);
    ui->dataSizeLabel_2->setText(tabDatas[i]->lastTableMessage);
    ui->tableDBNameLabel->setText(tabDatas[i]->lastTableDBName);

    executionTimer.stop();

    if(dc->data.exported || dc->data.exporting)
    {

        if(dc->data.saveRowSize >0)
            ui->exportProgressBar->setMaximum(dc->data.saveRowSize);
        else
            ui->exportProgressBar->setMaximum(1);
        ui->exportProgressBar->setValue(dc->data.saveRowsDone);
    }
    else
    {

        if(dc->data.tbldata.size()>0 && dc->data.tbldata[0].size() > 0 )
            ui->exportProgressBar->setMaximum(dc->data.tbldata[0].size());
        else
            ui->exportProgressBar->setMaximum(1);
        ui->exportProgressBar->setValue(0);

    }
    if(!dc->executing && !dc->data.exporting)
    {
        ui->pushButton_3->show();
        ui->stopLoadingQueryButton->hide();
        UpdateTable();
        if(dc->data.exported)
            on_exportDone();

        queryExecutionState = tabDatas[i]->lastQueryState;

    }
    else if(dc->data.exported && !dc->data.exporting )
        on_exportDone();
    else
    {
        if(dc->executing)
        {
            ui->tableWidget->clear();
        }

        ui->pushButton_3->hide();
        ui->stopLoadingQueryButton->show();
        executionTimer.setSingleShot(false);
        executionTimer.setTimerType(Qt::CoarseTimer);
        executionTimer.setInterval(15);
        executionTimer.start();
    }



}
void LoaderWidnow::on_tabWidget_tabBarDoubleClicked(int index)
{
    bool ok = true;
    QString newName = QInputDialog::getText (
        this, tr ("Change Name"),
        tr ("Insert New Tab Name"),
        QLineEdit::Normal,
        ui->tabWidget->tabText(index),
        &ok);

    if (ok) {
        ui->tabWidget->setTabText(index, newName);
        on_workspaceLineEdit_textChanged(ui->tabWidget->tabText(ui->tabWidget->currentIndex()));
    }

}
void LoaderWidnow::on_pushButton_4_clicked()
{// add tab
    QWidget* wg = new QWidget();
    wg->setWhatsThis(QVariant(_addtabcounter).toString());

    ui->tabWidget->addTab(wg,"New tab" + QVariant(_addtabcounter).toString());
    ui->tabWidget->setTabWhatsThis(ui->tabWidget->tabBar()->count()-1,QVariant(_addtabcounter).toString());


    tabDatas.push_back(new tabdata());
    tabDatas.back()->Name = "New tab" + QVariant(_addtabcounter).toString();
    tabDatas.back()->Id = QVariant(_addtabcounter).toString();

    tabDatas.back()->cd = new CodeEditor();

    ui->edit_layout->addWidget(tabDatas.back()->cd);
    tabDatas.back()->cd->hide();
    ui->tabWidget->setCurrentIndex(_addtabcounter);
    _addtabcounter++;
}


void LoaderWidnow::updateMisc()
{
    cd->updateMisc();
}
void LoaderWidnow::sendMail(QString host, QString Sender, QString SenderName, QStringList to,QStringList cc, QString Subject, QString messageText, QStringList attachments, QStringList pictutes)
{
    qDebug() << "sending mail" <<host << Sender <<SenderName << to << cc << Subject << messageText << attachments;
    auto server = new SimpleMail::Server;


    server->setHost(host);
    server->setConnectionType(SimpleMail::Server::TcpConnection);



    server->setAuthMethod(SimpleMail::Server::AuthNone);

    SimpleMail::MimeMessage message;
    message.setSender(SimpleMail::EmailAddress(Sender, SenderName));
    for(auto x : to)
    {
        qDebug() << "send to " <<  x;
        message.addTo(SimpleMail::EmailAddress(x));
    }
    if(!(cc.size()==1 && cc[0].trimmed() == ""))
        for(auto x : cc)
        {
            qDebug() << "copy to " <<  x;
            message.addCc(SimpleMail::EmailAddress(x));
        }

    message.setSubject(Subject);



    auto texthtml = std::make_shared<SimpleMail::MimeHtml>();

    QString html_text = "<p>"+messageText +"</p> ";

    std::vector<std::shared_ptr<SimpleMail::MimeAttachment>> attachedimages;
    int image_iter = 0;
    for(auto x : pictutes)
    {
        image_iter++;
        attachedimages.push_back(std::make_shared<SimpleMail::MimeAttachment>(std::make_shared<QFile>(documentsDir + "/" + x)));
        attachedimages.back()->setContentId(("image00" + QVariant(image_iter).toString()).toStdString().c_str());
        html_text += "<img src=\"cid:image00" + QVariant(image_iter).toString() +"\"> ";
    }
    qDebug() << "setting text";

    texthtml->setText(html_text );


    message.addPart(texthtml);


    for(auto x : attachedimages)
        message.addPart(x);

    if(!(attachments.size()==1 && attachments[0].trimmed() == ""))
        for(auto x : attachments)
        {
            qDebug() << "adding attachment " << documentsDir + x;
            message.addPart(std::make_shared<SimpleMail::MimeAttachment>( std::make_shared<QFile>(documentsDir + x)));
        }
    // Now we can send the mail
    SimpleMail::ServerReply *reply = server->sendMail(message);
    QObject::connect(reply, &SimpleMail::ServerReply::finished, [reply] {
        qDebug() << "ServerReply finished" << reply->error() << reply->responseText();
        reply->deleteLater();// Don't forget to delete it

    });

    qDebug() << "Message sent";
}


// qml test button
inline QQmlApplicationEngine* TestqmlEngine = nullptr;
void LoaderWidnow::on_pushButton_pressed()
{

    TestqmlEngine->load("DBLoadScript.qml");
}
// token processor test
void LoaderWidnow::on_pushButton_2_pressed()
{
    QString text = cd->toPlainText();
    QDir directory(documentsDir + "/" + "sqlBackup");
    QStringList strl = directory.entryList();



    tokenProcessor.tokens.clear();
    tokenProcessor.uniqueTokens.clear();
    tokenProcessor.ds.data.clear();
    tokenProcessor.freqs.clear();
    int fileid = 0;
    for(auto x : strl)
    {
        QFile f(documentsDir +"/sqlBackup/" + x);
        f.open(QFile::OpenModeFlag::ReadOnly);
        text = f.readAll();
        tokenProcessor.processText(text);
        tokenProcessor.addFrequencies();
        qDebug() <<fileid++;
    }

    for(auto x : tokenProcessor.freqs)
        for(auto y : x.second)
        {
            tokenProcessor.ds.SetProperty(x.first,y.first,y.second);
            tokenProcessor.uniqueTokens[y.first] = y.first;
        }

    tokenProcessor.ds.Save((documentsDir + "/" +"FrequencyMaps/test2.txt"));
    QFile fl ((documentsDir + "/" +"FrequencyMaps/tokens.txt"));
    fl.open(QFile::OpenModeFlag::WriteOnly);
    int i =0;
    fl.write("tokens\n");
    fl.write("{\n");

    for(auto a : tokenProcessor.uniqueTokens)
    {
        fl.write((" "+QVariant(i).toString() + " " + a + "\n").toLocal8Bit());
        i++;
    }
    fl.write("}\n");
    fl.close();
}



bool _tmp_drag_drop_is_file = false;

void LoaderWidnow::dragEnterEvent(QDragEnterEvent * evt)
{
    _tmp_drag_drop_is_file = false;
    const QMimeData *mimeData = evt->mimeData();
    qDebug()<<mimeData->formats();


    if(mimeData->hasFormat("application/x-qt-windosheet-mime;value=\"Csv\"") || mimeData->hasFormat("application/x-qt-windows-mime;value=\"Csv\""))// a part of excel table (excel table selection)
    {
        evt->accept();
    }
    if(mimeData->hasFormat("application/x-qt-windosheet-mime;value=\"FileContents\"") || mimeData->hasFormat("application/x-qt-windows-mime;value=\"FileContents\""))// if file is dropped from microsoft app, like attachment from Outlook
    {
        evt->accept();
    }
    if(evt->mimeData()->hasUrls())
        evt->accept();
}
void LoaderWidnow::dropEvent(QDropEvent * evt)
{


    dc->data.silentExcelImport=false;
    const QMimeData *mimeData = evt->mimeData();



    dc->executionStart = QDateTime::currentDateTime();
    if(mimeData->hasFormat("application/x-qt-windosheet-mime;value=\"Csv\"")|| mimeData->hasFormat("application/x-qt-windows-mime;value=\"Csv\""))// if file is dropped from microsoft app, like attachment from Outlook
    {
        //
        _tmp_drag_drop_is_file = true;
        QFile fl(documentsDir + "/tmpDragDropFile.xlsx");
        if(fl.open(QFile::OpenModeFlag::WriteOnly))
        {
            QString str = "";
            if(mimeData->hasFormat("application/x-qt-windosheet-mime;value=\"Csv\""))
                str = QString().fromLocal8Bit(mimeData->data("application/x-qt-windosheet-mime;value=\"Csv\""));
            else if (mimeData->hasFormat("application/x-qt-windows-mime;value=\"Csv\""))
                str = QString().fromLocal8Bit(mimeData->data("application/x-qt-windows-mime;value=\"Csv\""));
            QString resultcsvstr = "";
            int i=0;
            while(i<str.size() && str[i] != QChar('\x00'))
            {
                resultcsvstr += str[i++];
            }
            fl.write(resultcsvstr.toUtf8());
            qDebug()<<"written " + documentsDir + "/tmpDragDropFile.xlsx";
        }
        fl.close();
        if (dc->data.ImportFromCSV(documentsDir + "/tmpDragDropFile.xlsx",';',true))
        {
            dc->executionEnd = QDateTime::currentDateTime();
            UpdateTable();
            ui->tableDBNameLabel->setText("Imported from csv");
        }

    }
    else if(mimeData->hasUrls())
    {
        auto urls = mimeData->urls();
        foreach(auto url, urls) {
            QString str = url.toString();

            if(str.startsWith("file:///"))
                str.replace("file:///","");
            if(dc->data.ImportFromExcel(str,0,0,0,0,true))
            {
                dc->executionEnd = QDateTime::currentDateTime();
                UpdateTable();
                ui->tableDBNameLabel->setText("Imported from Excel");
            }
            else if (dc->data.ImportFromCSV(str,';',true))
            {
                dc->executionEnd = QDateTime::currentDateTime();
                UpdateTable();
                ui->tableDBNameLabel->setText("Imported from csv");
            }

        }
    }
    else if(mimeData->hasFormat("application/x-qt-windosheet-mime;value=\"FileContents\"")|| mimeData->hasFormat("application/x-qt-windows-mime;value=\"FileContents\""))// if file is dropped from microsoft app, like attachment from Outlook
    {
        _tmp_drag_drop_is_file = true;
        QFile fl(documentsDir + "/tmpDragDropFile.xlsx");
        if(fl.open(QFile::OpenModeFlag::WriteOnly))
        {
            if(mimeData->hasFormat("application/x-qt-windosheet-mime;value=\"FileContents\""))
                fl.write(mimeData->data("application/x-qt-windosheet-mime;value=\"FileContents\""));
            else if (mimeData->hasFormat("application/x-qt-windows-mime;value=\"FileContents\""))
                fl.write(mimeData->data("application/x-qt-windows-mime;value=\"FileContents\""));

            qDebug()<<"written " + documentsDir + "/tmpDragDropFile.xlsx";
        }
        fl.close();

        if(dc->data.ImportFromExcel(documentsDir + "/tmpDragDropFile.xlsx",0,0,0,0,true))
        {
            dc->executionEnd = QDateTime::currentDateTime();
            UpdateTable();
            ui->tableDBNameLabel->setText("Imported from Excel");
        }
        else if (dc->data.ImportFromCSV(documentsDir + "/tmpDragDropFile.xlsx",';',true))
        {
            dc->executionEnd = QDateTime::currentDateTime();
            UpdateTable();
            ui->tableDBNameLabel->setText("Imported from csv");
        }

    }
    dc->executionEnd = QDateTime::currentDateTime();
    dc->data.silentExcelImport=true;
}




void LoaderWidnow::splitColumn()
{
    int sizebuff = dc->data.tbldata.size();

    if(dc->data.tbldata.size()>0)
    {
        QString columnname = "";
        QString splitstr = ",";
        int columnid = 0;

        QInputDialog id(loadWind);
        id.setLabelText("Select column");
        id.setComboBoxItems(dc->data.headers);
        bool ok = id.exec();
        if(!ok)
            return;
        columnname = id.textValue();

        for(int i=0;i < dc->data.headers.size();i++)
            if(dc->data.headers[i] == columnname)
            {
                columnid = i;
            }



        splitstr = QInputDialog::getText (
            this, tr ("Devide by"),
            tr ("Devide each string by text"),
            QLineEdit::Normal,
            ",",
            &ok);

        std::vector<QStringList> v_strl;
        int maxsize = 0;
        for(int i=0;i<dc->data.tbldata[columnid].size();i++)
        {
            v_strl.push_back(dc->data.tbldata[columnid][i].split(splitstr));
            if(v_strl.back().size()>maxsize)
                maxsize = v_strl.back().size();
        }

        dc->data.tbldata.resize(sizebuff + maxsize);
        dc->data.headers.resize(sizebuff + maxsize);


        for(int i=sizebuff;i < sizebuff + maxsize;i++)
        {
            dc->data.tbldata[i].resize(dc->data.tbldata[columnid].size());
            dc->data.headers[i] = "Column_" + QVariant(i-sizebuff).toString();
            dc->data.maxVarTypes.push_back(QMetaType::QString);
            dc->data.typecount.emplace_back();
            while(dc->data.typecount.back().size()<=QMetaType::QString)
                dc->data.typecount.back().push_back(0);
        }
        int it = 0;
        for(auto x : v_strl)
        {
            for (int i=0;i < x.size() && i < maxsize;i++)
            {
                dc->data.tbldata[i + sizebuff][it] = x[i].trimmed();
            }
            it++;

        }
    }
    UpdateTable();
}


void LoaderWidnow::addNumeratorColumn()
{

    if(dc->data.tbldata.size() > 0)
    {
        int sizebuff = dc->data.tbldata.size();

        dc->data.tbldata.resize(sizebuff + 1);
        dc->data.headers.resize(sizebuff + 1);
        dc->data.headers.back() = "ID";
        dc->data.maxVarTypes.push_back(QMetaType::Double);
        dc->data.typecount.emplace_back();
        while(dc->data.typecount.back().size()<=QMetaType::Double)
            dc->data.typecount.back().push_back(0);
        dc->data.typecount.back().back() = dc->data.tbldata[0].size();

        for(int i=0;i < dc->data.tbldata[0].size();i++)
        {
            dc->data.tbldata.back().push_back(QVariant(i).toString());
        }
        UpdateTable();
    }
}

void LoaderWidnow::deleteDublicates()
{

    if(dc->data.tbldata.size() > 0)
    {
        int sizebuff = dc->data.tbldata.size();

        std::map<QString,int> rowRepeatCount;
        int i=0;
        while(i < dc->data.tbldata[0].size())
        {
            QString rowStr = "";
            for(int a=0;a< dc->data.tbldata.size();a++)
            {
                rowStr += dc->data.tbldata[a][i];
            }

            rowRepeatCount[rowStr];

            if(rowRepeatCount[rowStr]<=0)
            {// first row
                rowRepeatCount[rowStr] = 1;
                i++;
            }
            else
            {//delete row

                for(int a=0;a< dc->data.tbldata.size();a++)
                {
                    dc->data.tbldata[a].removeAt(i);
                }
            }
        }



        UpdateTable();
    }
}

void LoaderWidnow::levensteinJoin()
{
    qDebug() <<"levensteinJoin()";


    int sizebuff = dc->data.tbldata.size();

    if(dc->data.tbldata.size()>0)
    {
        QString source_columnname = "";
        QString data_columnname = "";
        QString group_columnname = "";
        int source_columnid = 0;
        int data_columnid = 0;
        int group_columnid = -1;

        QInputDialog id(loadWind);
        id.setLabelText("Select source column");
        id.setComboBoxItems(dc->data.headers);
        bool ok = id.exec();
        if(!ok)
            return;
        source_columnname  = id.textValue();

        for(int i=0;i < dc->data.headers.size();i++)
            if(dc->data.headers[i] == source_columnname )
            {
                source_columnid = i;
            }

        QInputDialog idd(loadWind);
        idd.setLabelText("Select column to join with");
        idd.setComboBoxItems(dc->data.headers);
        ok = idd.exec();
        if(!ok)
            return;
        data_columnname  = idd.textValue();

        for(int i=0;i < dc->data.headers.size();i++)
            if(dc->data.headers[i] == data_columnname )
            {
                data_columnid = i;
            }



        idd.setLabelText("Select column to group with");
        QStringList strl = dc->data.headers;
        strl.push_front("none");
        idd.setComboBoxItems(dc->data.headers);
        ok = idd.exec();
        if(!ok)
            return;
        group_columnname  = idd.textValue();

        for(int i=0;i < dc->data.headers.size();i++)
            if(dc->data.headers[i] == group_columnname )
            {
                group_columnid = i;
            }



        std::map<QString,std::map<QString,int>> uniqueDataStrings;
        std::map<QString,std::map<QString,int>> uniqueSourceToDataStringCosts;
        std::map<QString,std::map<QString,QString>> uniqueSourceStrings;

        for(int i=0;i < dc->data.tbldata[data_columnid].size();i++)
        {
            QString groupText = "t";
            if(group_columnid!=-1)
            {
                groupText = dc->data.tbldata[group_columnid][i];
            }
            uniqueDataStrings[groupText][dc->data.tbldata[data_columnid][i]] += 1;

        }

        for(int i=0;i < dc->data.tbldata[source_columnid].size();i++)
        {
            QString groupText = "t";
            if(group_columnid!=-1)
            {
                groupText = dc->data.tbldata[group_columnid][i];
            }
            uniqueSourceStrings[groupText][dc->data.tbldata[source_columnid][i]] = "";
            uniqueSourceToDataStringCosts[groupText][dc->data.tbldata[source_columnid][i]] = 0;

        }



        int wrong_cost = 10;
        int move_cost = 3;
        int add_cost = 1;
        int shorten_cost = 8;

        int itercount = 0;
        for(auto groupText : uniqueSourceStrings)
        {
            for(auto word : groupText.second)
            {
                // select a word with minimal distance for every sorce word
                float min_diff = 1000000000;
                QString mindiffWord = "";

                for(auto k : uniqueDataStrings[groupText.first])
                {
                    if(k.first == word.first)
                        continue;
                    long int diff = 0;
                    int last_i = 0;
                    float modifier = 100;

                    for(int i=0;i<word.first.size();i++)
                    {
                        int mdiff = wrong_cost;
                        for(int a = 0 ; a<k.first.size();a++)
                        {
                            if(k.first[a].toLower()==word.first[i].toLower() && move_cost * abs(i-a)<mdiff)
                                mdiff = move_cost * abs(i-a);
                        }
                        diff+=mdiff;
                        last_i = i;
                    }
                    if(k.first.size()<word.first.size())
                        diff+=abs(k.first.size()-word.first.size()) * shorten_cost;
                    else
                        diff+=abs(k.first.size()-word.first.size()) * add_cost;
                    diff *= modifier;
                    if(diff<min_diff)
                    {
                        min_diff = diff;
                        mindiffWord = k.first;
                    }

                }

                // save to word
                uniqueSourceStrings[groupText.first][word.first] = mindiffWord;
                uniqueSourceToDataStringCosts[groupText.first][word.first] = min_diff;
                itercount++;
            }
        }


        dc->data.tbldata.resize(sizebuff + 2);
        dc->data.tbldata[dc->data.tbldata.size()-1].reserve(dc->data.tbldata[source_columnid].size());
        dc->data.tbldata[dc->data.tbldata.size()-2].reserve(dc->data.tbldata[source_columnid].size());
        dc->data.headers.resize(sizebuff + 2);

        dc->data.headers[dc->data.headers.size()-2] = "lev_Cost";
        dc->data.maxVarTypes.push_back(QMetaType::Double);
        dc->data.typecount.emplace_back();
        while(dc->data.typecount.back().size()<=QMetaType::Double)
            dc->data.typecount.back().push_back(0);

        dc->data.headers.back() = "lev_Text";
        dc->data.maxVarTypes.push_back(QMetaType::QString);
        dc->data.typecount.emplace_back();
        while(dc->data.typecount.back().size()<=QMetaType::QString)
            dc->data.typecount.back().push_back(0);



        for(int i=0;i < dc->data.tbldata[source_columnid].size();i++)
        {
            QString groupText = "t";
            if(group_columnid!=-1)
            {
                groupText = dc->data.tbldata[group_columnid][i];
            }
            dc->data.tbldata[dc->data.tbldata.size()-1].push_back(uniqueSourceStrings[groupText][dc->data.tbldata[source_columnid][i]]);
            dc->data.tbldata[dc->data.tbldata.size()-2].push_back(QVariant(uniqueSourceToDataStringCosts[groupText][dc->data.tbldata[source_columnid][i]]).toString());
        }


    }
    UpdateTable();

}


inline float NN_min = 1000000;
inline float NN_max = -1000000;
void LoaderWidnow::on_nnTestLearn_pressed()
{


    qDebug() << "on_nnTestLearn_pressed undefined";


    if(dc->data.tbldata.size() <=0)
        return;

    std::vector<float> input;
    std::vector<float> output;
    input.resize(dc->data.tbldata[0].size());
    output.resize(dc->data.tbldata[0].size());


    for(int i=0;i < dc->data.tbldata[0].size();i++)
    {
        input[i] = ((float)i)/(float) dc->data.tbldata[0].size() ;
        bool ok = false;
        output[i] = dc->data.tbldata[0][i].toFloat(&ok);
        qDebug() << output[i] << " = " << dc->data.tbldata[0][i].toFloat(&ok);
        if (NN_min > output[i])
            NN_min = output[i];

        if (NN_max < output[i])
            NN_max = output[i];

    }
    for(int i=0;i <output.size();i++)
    {
        output[i] = (output[i] - NN_min*0.5f) / (NN_max*2.0f - NN_min*0.5f);// put into range(0,1) * 0.5f
        qDebug() << output[i];
    }

    int datasizeBuff = dc->data.tbldata[0].size();

    for(int i=0;i<=5000;i++)
    {
        float cst = nn.lastCost;
        if(cst > 1.0f)
            nn.h = 0.0001f;
        else
            nn.h = 0.0001f * cst;

        float costbuff = nn.lastCost;

        nn.lastCost = 0;
        for(int iter = 0;iter < input.size();iter++)
        {
            float val = (((float)iter)/(float)input.size());
            nn.BackPropagate(&val,&output[iter],0.1f);
            nn.lastCost += nn.Cost(&val,&output[iter],1);
        }



        if(i%5000 == 0)
        {
            int datasize = datasizeBuff * 2.0f;


            // nn.UpdateWeightsAndBiases();
            //nn.ReadGPUWeightsBiases();

            TableData data;
            data.headers.clear();
            data.headers.resize(3);
            data.headers[0] = "Source";
            data.headers[1] = "iteration";
            data.headers[2] = "value";
            data.tbldata.resize(3);
            data.tbldata[0].resize(datasize);
            data.tbldata[1].resize(datasize);
            data.tbldata[2].resize(datasize);

            for(int a=0;a < datasize;a++)
            {
                float input = (((float)a)/(float)datasize) * 2.0f ;
                nn.Run(&input);

                QString str = QVariant(a).toString();
                while(str.size()<10)
                    str = "0" + str;
                data.tbldata[0][a] = "iteration"  + QVariant(i).toString();
                data.tbldata[1][a] = str;
                data.tbldata[2][a] = QVariant((nn.outputs[0] * (NN_max*2.0f - NN_min*0.5f)) + NN_min*0.5f).toString();
            }
            qDebug() << "here";
            data.FixTypeInfo();
            UpdateTable();
            data.ExportToSQLiteTable("NN_Learn_Iteration"  + QVariant(i).toString());
        }
        qDebug() << "end cost " <<  cst;
    }


    /*
    std::vector<float> input(nn.sizein);
    std::vector<float> output(nn.sizeout);


    // uniqueTokenIdMap.clear();

    // int uniqueTokenIdMap_iter =  0;
    // for(auto x : uniqueTokens.data["tokens"])
    // {
    //     uniqueTokenIdMap[uniqueTokenIdMap_iter] = x.first;
    // }


    for(int i=0;i < nn.sizein;i++)
    {
        input[i]= 0;
        output[i]= 0;
    }

    int input_iter = 0;
    QStringList strl = cd->GetTokensUnderCursor();
    qDebug() << "gor tokems";
    for(auto in_tok : strl)
    {
        if(uniqueTokens.data["tokens"].count(in_tok)>0)
            input[uniqueTokens.data["tokens"][in_tok].toInt()] += (float)(strl.size() - input_iter)/(float)strl.size();
        input_iter++;
    }

    qDebug() << "runing" << input.size() << nn.sizein << uniqueTokens.data["tokens"].size();
    nn.Run(input.data());
    qDebug() << "ran";


    int maxI = 0;
    for(int i=0;i < nn.sizeout;i++)
    {
        if(nn.outputs[maxI]<nn.outputs[i] )
            maxI = i;
    }

    QString maxToken = "";
    int map_iter = 0;
    for(auto x : uniqueTokens.data["tokens"])
    {
        if(map_iter == maxI)
        {
            maxToken = x.first;
            break;
        }
        map_iter++;
    }
    cd->textCursor().insertText(maxToken);


    */
}

int iteration_counter = 0;
bool AI_CyberDimentia = true;
int correct_word_count = 0;


// used to split text
#include "sqlSubfunctions.h"



void LoaderWidnow::on_nnTestRun_pressed()
{

    // Small toy corpus
    QFile fl("C:/Users/pavel.kholkovskiy/Documents/DBLoader/sqlBackup/2025_08_22_09_53_23 closed tab WorkSpace0.sql.sql");

    QString corpus;
    if(fl.open(QFile::OpenModeFlag::ReadOnly))
        corpus= fl.readAll();

    // QString sqlFileContent = "SELECT * FROM table WHERE id = '123'; -- comment";
    // QString cleaned = clean_text(corpus);
    // qDebug() << "Cleaned SQL:" << cleaned;
    // cleaned = clean_text(sqlFileContent);
    // qDebug() << "Cleaned SQL:" << cleaned;

    // BPE bpe;
    // bpe.train(corpus, /*numMerges=*/500);

    // QString input = QStringLiteral("select * from service where lower(serv_name) like 'asdzxc'");

    // QStringList tokens = bpe.encode(input);

    // qDebug()<< "Input: " << input;
    // qDebug()<< "Tokens (" << tokens.size() << "): [";
    // for (int i = 0; i < tokens.size(); ++i) {

    //     qDebug() << (i ? ", " : "") << tokens.at(i);
    // }
    // qDebug()<< "]\n";

    // const QString decoded = bpe.decode(tokens);
    // qDebug() << "Decoded: " << decoded;


    return;
/*
    nn.inputs = new float[nn.sizein];
    nn.outputs = new float[nn.sizeout];
    for (int i=0; i<nn.sizeout; i++) {
        nn.outputs[i] = 0;
    }

    for (int i=0; i<nn.sizein; i++) {
        nn.inputs[i] = 0;//rand()%10 *0.1f;
    }

    for (int i=0; i<nn.NN_Size; i++) {
        nn.biases[i] = 0.0f;
    }



    DataStorage new_uniqueTokens;
    new_uniqueTokens.data["tokens"];

    std::vector<QString> lines;
    std::vector<QString> trg_tokens;
    std::vector<float> input_buff;
    std::vector<float> output_buff;
    std::vector<float> result_output_buff;
    std::vector<float> ranked_output_buff;
    int datasAmount = 0;


    std::vector<float> input(nn.sizein);
    std::vector<float> output(nn.sizeout);
    std::vector<float> ranked_output(nn.sizeout);

    uniqueTokenIdMap.clear();

    int uniqueTokenIdMap_iter =  0;
    for(auto x : uniqueTokens.data["tokens"])
    {
        uniqueTokenIdMap[uniqueTokenIdMap_iter] = x.first;
    }


    int maxiter = 0;
    for(auto obj : tokenProcessor.ds.data)
    {
        maxiter++;
        // if(maxiter > 20'000)
        // {
        //     break;
        // }


        int line_size = obj.first.split(" ").size();
        if(line_size>0 && obj.first.size() > 10 &&  obj.first.count(";") == 0 && HasLetters(obj.first))
        {

            for(int i=0;i < nn.sizein;i++)
            {
                input[i]= 0;
                output[i]= 0;
            }

            int input_iter = 0;
            bool brek = false;
            int non_letter_counter = 0;
            QStringList intoklist;
            for(auto in_tok : obj.first.split(" "))
            {
                if(!HasLetters(in_tok))
                    non_letter_counter ++;
                else
                    non_letter_counter =0;

                if(non_letter_counter >=3 || uniqueTokens.data["tokens"].count(in_tok) < 1)
                {
                    brek = true;
                    break;
                }

                if(uniqueTokens.data["tokens"].count(in_tok)>0)
                {
                    input[uniqueTokens.data["tokens"][in_tok].toInt()] += ((float)(input_iter)/(float)line_size) * ((float)(input_iter)/(float)line_size);
                    intoklist.push_back(in_tok);
                    input_iter++;
                }
            }
            if(brek)
                continue;


            QString trgToken = "";
            float trgToken_maxval = 0;
            int outputSize = 0;
            for(auto dat : obj.second)
            {
                outputSize++;
                bool ok = false;
                if(uniqueTokens.data["tokens"].count(dat.first)>0)
                {
                    output[uniqueTokens.data["tokens"][dat.first].toInt()] += dat.second.toFloat(&ok);
                    intoklist.push_back(dat.first);
                }

                if(trgToken_maxval<dat.second.toDouble())
                {
                    trgToken_maxval = dat.second.toDouble();
                    trgToken = dat.first;
                }
            }
            if(outputSize<10 || trgToken_maxval < 50.0f)
                continue;

            for(auto x : intoklist)
            {

                if(uniqueTokens.data["tokens"].count(x)<=0)
                {
                    qDebug() << "not found "<< x;
                    return;
                }

                new_uniqueTokens.data["tokens"][x];
            }
            intoklist.clear();


            ranked_output.resize(output.size());

            int max_out_id = 0;
            for(int i = 0; i< output.size();i++)
            {
                if(output[max_out_id] < output[i])
                    max_out_id = i;
            }
            for(int i = 0; i< output.size();i++)
            {
                if(i == max_out_id)
                     ranked_output[i] = 1.0f;
                else
                     ranked_output[i] = 0;
                //ranked_output[i] = output[i] / output[max_out_id];
            }

            //nn.Run(input.data());


            for(int i=0; i < input.size();i++)
                input_buff.push_back(input[i]);
            for(int i=0; i < ranked_output.size();i++)
                output_buff.push_back(ranked_output[i]);
            datasAmount++;
            lines.push_back(obj.first);
            trg_tokens.push_back(trgToken);
            qDebug() <<datasAmount << input_buff.size() << output_buff.size();//<< "       nn.outputs[maxI] = " << nn.outputs[maxI] << "/" << output[maxI] <<"  "<<obj.first <<  maxToken  << "   trgToken: " << trgToken << 1;
            if(datasAmount >=100)
                break;
        }
    }

    result_output_buff.reserve(output_buff.size());
    ranked_output_buff.resize(output_buff.size());

    qDebug() << "pushing inputs";

    int tmp_buffer = datasAmount;
    iteration_counter = 0;

    float lastbest = 0.0f;

    while (iteration_counter < 500)
    {
        result_output_buff.clear();
        for(int i = 0; i < datasAmount;i++)
        {
            nn.BackPropagateGPU(&input_buff[i * nn.sizein],&output_buff[i * nn.sizeout],0.001f);
            for(int a = 0; a < nn.sizeout; a++)
                result_output_buff.push_back(nn.outputs[a]);
        }
        datasAmount = tmp_buffer;
        qDebug() << "done, loading data";


        nn.ReadGPUWeightsBiases();
        correct_word_count = 0;
        for(int a =0;a < datasAmount && a < lines.size() && a < trg_tokens.size(); a++)
        {
            for(int i = 0; i < nn.sizein && i + a * nn.sizein < output_buff.max_size()&& i + a * nn.sizein < result_output_buff.max_size(); i ++)
            {
                nn.outputs[i] = result_output_buff[i + a * nn.sizein];
                output[i] = output_buff[i + a * nn.sizein];
            }





            std::vector<float> nnoutputDiffs(nn.sizeout);

            int maxI = 0;
            for(int i=0;i < nn.sizeout;i++)
            {
                nnoutputDiffs[i] = output[i] - nn.outputs[i];


                if(nn.outputs[maxI]<nn.outputs[i] )
                    maxI = i;
            }

            //nn.ApplyDiff(input.data(),nnoutputDiffs.data(),0.05f);

            QString maxToken = "";
            int map_iter = 0;
            for(auto x : uniqueTokens.data["tokens"])
            {
                if(map_iter == maxI)
                {
                    maxToken = x.first;
                    break;
                }
                map_iter++;
            }
            qDebug() << iteration_counter <<a  << "nn.outputs[maxI] = " << nn.outputs[maxI] << "/" << output[maxI] <<"  "<< lines[a] + "(" + maxToken  + " | " + trg_tokens[a] + ")";
            if(maxToken == trg_tokens[a])
            {
                correct_word_count++;
            }
        }
        qDebug() << iteration_counter << "correct count: " << correct_word_count << "/" << datasAmount << "(" << ((float)correct_word_count/(float)datasAmount * 100) << "%) +" << ((float)correct_word_count/(float)datasAmount * 100) - lastbest << "%" << "AI_CyberDimentia = " << AI_CyberDimentia;
        if(((float)correct_word_count/(float)datasAmount * 100) - lastbest > 1.5f )
        {
            lastbest = ((float)correct_word_count/(float)datasAmount * 100);

            if(((float)correct_word_count/(float)datasAmount * 100) > 30.0f)
                nn.SaveTo((documentsDir + "/nntest_"+ QVariant(lastbest).toString() +".nn").toStdString());
        }
        if ((correct_word_count >= datasAmount * 0.40f))//
            AI_CyberDimentia = false;
        // if(iteration_counter%5 == 0)
        //     nn.SaveTo((documentsDir + "/nntest_"+ QVariant(iteration_counter).toString() +".nn").toStdString());
        iteration_counter++;
    }
    //nn.SaveTo((documentsDir + "/nntest_"+ QVariant("result").toString() +".nn").toStdString());



    /* OpenXLSX test bench. Saves pivot tables, but will requere  a lotof rewriting
    XLDocument doc1;
    doc1.open("./Demo04.xlsx");
    if(!doc1.isOpen())
        doc1.create("./Demo04.xlsx", XLForceOverwrite);


    auto wks1 = doc1.workbook().worksheet("Простыня");
    // wks1.setName("Простыня");

    // Cell values can be set to any Unicode string using the normal value assignment methods.

    // qDebug() << QDateTime::currentSecsSinceEpoch() / 86400.0f;

    // // Create an XLDateTime object from an Excel date number (e.g., 55.5 represents Feb 24, 1900, 12:00 PM)
    // OpenXLSX::XLDateTime dt( 45658.0);//QDateTime::currentSecsSinceEpoch() / 86400.0f + 86400.0f + 3600*3);

    // // Convert to std::tm structure
    // std::tm tmo = dt.tm();

    // // Print the time using asctime for human-readable format
    // qDebug() << "Time from XLDateTime: " << std::asctime(&tmo);

    // XLStyles style = doc1.workbook()
    // style.numberFormat().setFormat("yyyy-mm-dd"); // Customize as needed
    // wks1.cell("A1").setCellFormat();

    ;// Or create a new style

    // style.numberFormat().setFormat("yyyy-mm-dd"); // Customize as needed
    // qDebug() << wks1.cell("A1").cellFormat();

    ;

    //for(int i=0;i < doc1.styles().cellFormats().count();i++)
    //{
    //    qDebug() << doc1.styles().numberFormats().numberFormatById(i).summary() <<doc1.styles().numberFormats().numberFormatById(i).formatCode() << doc1.styles().numberFormats().numberFormatById(i).numberFormatId();
    //}

    //XLCellFormats & cellFormats = doc1.styles().cellFormats();
    qDebug() << "doc1.styles().cellFormats().count() " << doc1.styles().cellFormats().count();

    for(int i=0;i < doc1.styles().cellFormats().count();i++)
    {
        qDebug() << i << doc1.styles().cellFormats().cellFormatByIndex(i).summary() << doc1.styles().cellFormats().cellFormatByIndex(i).numberFormatId()  ;
        if(doc1.styles().cellFormats().cellFormatByIndex(i).numberFormatId() > 0)
        {
            uint32_t numfmtid = doc1.styles().cellFormats().cellFormatByIndex(i).numberFormatId();
            // qDebug() << doc1.styles().numberFormats().numberFormatById(14).formatCode();
            // qDebug() << doc1.styles().numberFormats().numberFormatById(14).summary();
            // si = i;
        }
    }

    XLCellFormats & cellFormats = doc1.styles().cellFormats();

    XLStyleIndex DateTimeStyle = cellFormats.create();

    XLStyleIndex nfi = doc1.styles().numberFormats().create();
    OpenXLSX::XLNumberFormat nm = doc1.styles().numberFormats().numberFormatByIndex(nfi);
    nm.setFormatCode("DD.MM.YYYY HH:mm:SS");
    cellFormats.cellFormatByIndex(DateTimeStyle).setApplyNumberFormat(true);
    cellFormats.cellFormatByIndex(DateTimeStyle).setNumberFormatId(nm.numberFormatId());





    // void ExcelSerialDateToDMY(int nSerialDate, int& nDay, int& nMonth, int& nYear)
    // {
    //     // Modified Julian to DMY calculation with an addition of 2415019
    //     int l  = nSerialDate + 68569 + 2415019;
    //     int n  = int(( 4 * l ) / 146097);
    //     l      = l - int(( 146097 * n + 3 ) / 4);
    //     int i  = int(( 4000 * ( l + 1 ) ) / 1461001);
    //     l      = l - int(( 1461 * i ) / 4) + 31;
    //     int j  = int(( 80 * l ) / 2447);
    //     nDay   = l - int(( 2447 * j ) / 80);
    //     l      = int(j / 11);
    //     nMonth = j + 2 - ( 12 * l );
    //     nYear  = 100 * ( n - 49 ) + i + l;
    // }


    // static qint64 msecs1904 = QDateTime(QDate(1904, 1, 1), QTime(0, 0)).toMSecsSinceEpoch();
    // static qint64 msecs1899 = QDateTime(QDate(1899, 12, 31), QTime(0, 0)).toMSecsSinceEpoch();

    // if (!is1904 && num > 60) // for mac os excel
    // {
    //     num = num - 1;
    // }

    // auto msecs = static_cast<qint64>(num * 1000 * 60 * 60 * 24.0 + 0.5);

    // if (is1904)
    //     msecs += msecs1904;
    // else
    //     msecs += msecs1899;

    // QDateTime dtRet = QDateTime::fromMSecsSinceEpoch(msecs);

    // // Remove one hour to see whether the date is Daylight
    // QDateTime dtNew = dtRet.addMSecs(-3600000); // issue102
    // if (dtNew.isDaylightTime()) {
    //     dtRet = dtNew;
    // }

    // double whole      = 0;
    // double fractional = std::modf(num, &whole);

    // if (num < double(1)) {
    //     // only time
    //     QTime t = dtRet.time();
    //     // return QVariant(t);
    // }

    // if (fractional == 0.0) {
    //     // only date
    //     QDate onlyDT = dtRet.date();
    //     // return QVariant(onlyDT);
    // }

    // // return QVariant(dtRet);







    wks1.cell(XLCellReference(1,1)).value() = "안녕하세요 세계!";
    wks1.cell(XLCellReference(1,2)).value() = "你好，世界!";
    wks1.cell(XLCellReference(1,3)).value() = "こんにちは 世界";
    wks1.cell(XLCellReference(1,4)).value() = "नमस्ते दुनिया!";
    wks1.cell(XLCellReference(1,5)).value() = "Привет, мир!";
    wks1.cell(XLCellReference(1,6)).value() = "Γειά σου Κόσμε!";

    wks1.cell(XLCellReference(40,7)).value() = 45032.2;
    wks1.cell(XLCellReference(40,7)).setCellFormat(DateTimeStyle);


    wks1.cell(XLCellReference(40,8)).value() = 45032.2;
    wks1.cell(XLCellReference(40,7)).setCellFormat(0);
    wks1.cell(XLCellReference(40,9)).value() = "45032.2";

    QDateTime dt = QDateTime::currentDateTime();

    double currentdt = (int(( 1461 * ( dt.date().year() + 4800 + int(( dt.date().month() - 14 ) / 12) ) ) / 4) +
        int(( 367 * ( dt.date().month() - 2 - 12 * ( ( dt.date().month() - 14 ) / 12 ) ) ) / 12) -
        int(( 3 * ( int(( dt.date().year() + 4900 + int(( dt.date().month() - 14 ) / 12) ) / 100) ) ) / 4) +
            dt.date().day() - 2415019 - 32075) + (dt.time().second() * 1.0  + dt.time().minute()*60.0 + dt.time().hour()*3600.0)/86400.0;
    wks1.cell(XLCellReference(45,8)).value() = currentdt;

    // Workbooks can also be saved and loaded with Unicode names
    doc1.save();
    doc1.saveAs(QString("фывячс.xlsx").toLocal8Bit().constData(), XLForceOverwrite);
    doc1.close();


    doc1.open(QString("фывячс.xlsx").toLocal8Bit().constData());
    if(!doc1.isOpen())
        doc1.create(QString("фывячс.xlsx").toLocal8Bit().constData(), XLForceOverwrite);

    if(!doc1.isOpen())
    {
        qDebug() << "doc2 is not opened";
        return;
    }
    qDebug() << "doc2 is opened";
    wks1 = doc1.workbook().worksheet("Простыня");
    if(!wks1.valid())
    {
        qDebug() << "couldnt read sheet name";
        return;
    }


    // The nowide::cout object is a drop-in replacement of the std::cout that enables console output of UTF-8, even on Windows.
    qDebug() << "Cell A1 (Korean)  : " << wks1.cell(XLCellReference("A1")).value().get<std::string>() << '\n';
    qDebug() << "Cell A2 (Chinese) : " << wks1.cell(XLCellReference("A2")).value().get<std::string>() << '\n';
    qDebug() << "Cell A3 (Japanese): " << wks1.cell(XLCellReference("A3")).value().get<std::string>() << '\n';
    qDebug() << "Cell A4 (Hindi)   : " << wks1.cell(XLCellReference("A4")).value().get<std::string>() << '\n';
    qDebug() << "Cell A5 (Russian) : " << wks1.cell(XLCellReference("A5")).value().get<std::string>() << '\n';
    qDebug() << "Cell A6 (Greek)   : " << wks1.cell(XLCellReference("A6")).value().get<std::string>() << '\n';


    cout << "\nNOTE: If you are using a Windows terminal, the above output may look like gibberish,\n"
            "because the Windows terminal does not support UTF-8 at the moment. To view to output,\n"
            "you can use the overloaded 'cout' in the boost::nowide library (as in this sample program).\n"
            "This will require a UTF-8 enabled font in the terminal. Lucinda Console supports some\n"
            "non-ASCII scripts, such as Cyrillic and Greek. NSimSun supports some asian scripts.\n\n";

    doc1.close();
    qDebug() << "on_nnTestRun_pressed undefined";*/
}
