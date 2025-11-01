#ifndef LOADERWIDNOW_H
#define LOADERWIDNOW_H

#include <QMainWindow>
#include <qbarseries.h>
#include <qbarset.h>
#include <qboxlayout.h>
#include <qchartview.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qdatetimeaxis.h>
#include <qfilesystemmodel.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlineseries.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qsplineseries.h>
#include <qthread.h>
#include <qtimer.h>
#include <qtreeview.h>
#include <qvalueaxis.h>
#include <QChart>
#include "replacedialog.h"

#include "NeuralNetwork.h"
#include "codeeditor.h"
#include "databaseconnection.h"
#include "replacewindow.h"


//  Launch options
inline QString launchOpenFileName = "";
inline QString launchIntervalParameter = "";
inline QString launchIntervalHour = "";
inline QString launchIntervalMinute = "";
inline bool launchOpenFile = false;
inline bool launchAutomation = false;
namespace Ui {
class LoaderWidnow;
}

class LoaderWidnow : public QMainWindow
{
    Q_OBJECT

public:
    explicit LoaderWidnow(QWidget *parent = nullptr);
    ~LoaderWidnow();


    QFileSystemModel* workspace_model = nullptr;
    QTreeView *workspace_tree = nullptr;

    CodeEditor* cd = nullptr;

    // connection
    DatabaseConnection* dc = nullptr;
    QThread** sqlexecThread = nullptr;
    bool disableAutoconnect = false;

    bool autolaunchTimerWindowVisible = false;
    QTimer autolaunchTimer;

    //Tabs
    QStringList tabNames;
    QStringList tabIds;
    struct tabdata
    {
        QThread* sqlexecThread = nullptr;
        DatabaseConnection dc;
        CodeEditor* cd = nullptr;
        QString Name = "tab";
        QString Id = "0";
        QString sql = "";
        QString workspaceName = "tmp";
        int lastQueryState = 0;
        QString lastMessage = "";
        QString lastTableMessage = "";
        QString lastTableDBName = "";
        int textposition = 0;
        int iconID = 0;
        ~tabdata()
        {
            if(!dc.executing && sqlexecThread != nullptr)
                delete sqlexecThread;
            dc.stopRunning();
        }
    };
    std::vector <tabdata*> tabDatas;
    int currentTabId = 0;
    bool _tab_failsave = true;

    // Query states
    int queryExecutionState = 0;
    QTimer executionTimer;

    //Workspace info/ history
    QString tmpSql = ""; // for history to return back
    QString LastWorkspaceName = "WorkSpace0";

    //subwindow states
    bool b_showHistoryWindow = false;
    bool b_showWorkspaceWindow = false;
    bool b_showIteratorWindow = true;

    // replacer info
    QString _replacePrev_What = "";
    QString _replacePrev_With = "";
    int _replacePrev_from = 0;
    int _replacePrev_to = 99999;

    struct graph_Window
    {
        QVBoxLayout graph_layout;
        QHBoxLayout SpinBox_layout;
        QHBoxLayout Labels_layout;
        QHBoxLayout Settings_layout;



        QComboBox groupBysb;
        QSpinBox groupSubstrBegin;
        QSpinBox groupSubstrEnd;
        QLabel cutfirst;
        QLabel cutlast;

        QCheckBox qroupDateYYYYCheckbox;
        QCheckBox qroupDateMMCheckbox;
        QCheckBox qroupDateDDCheckbox;
        QCheckBox qroupDatehhCheckbox;
        QCheckBox qroupDatemmCheckbox;
        QCheckBox qroupDatessCheckbox;

        QComboBox separateBysb;
        QComboBox dataColumnsb;
        QComboBox dataAggMethod;

        QComboBox graphTypeCB;
        QLabel graphTypeLabel;

        QLabel groupByLabel;
        QLabel separateByLabel;
        QLabel dataColumnLabel;

        QPushButton buildGraphButton;
        QPushButton saveAsPDFButton;
        QPushButton copyScriptHandle;
        QCheckBox showLabelsCheckBox;
        QCheckBox graphThemeCheckBox;

        QChartView cv;
        QChart chrt;
        QLineSeries ls;

        QVector<QLineSeries*> Straight_line_series;
        QVector<QSplineSeries*> line_series;
        QVector<QBarSet*> bar_sets;
        QBarSeries bs;

        QDateTimeAxis da;
        QValueAxis vax;
        QValueAxis vay;

        bool manual_override = false;
        int manual_groupColumn = 0;
        int manual_dataColumn = 0;
        int manual_separateColumn = 0;
        QString savename = "chart.png";

        void Init()
        {
            Settings_layout.addWidget(&cutfirst);

            Settings_layout.addWidget(&groupSubstrBegin);
            Settings_layout.addWidget(&cutlast);
            Settings_layout.addWidget(&groupSubstrEnd);

            Settings_layout.addWidget(&qroupDateYYYYCheckbox);
            Settings_layout.addWidget(&qroupDateMMCheckbox);
            Settings_layout.addWidget(&qroupDateDDCheckbox);
            Settings_layout.addWidget(&qroupDatehhCheckbox);
            Settings_layout.addWidget(&qroupDatemmCheckbox);
            Settings_layout.addWidget(&qroupDatessCheckbox);

            Settings_layout.addWidget(&dataAggMethod);

            SpinBox_layout.addWidget(&groupBysb);
            SpinBox_layout.addWidget(&separateBysb);
            SpinBox_layout.addWidget(&dataColumnsb);
            SpinBox_layout.addWidget(&graphTypeCB);

            graphTypeCB.addItem("Spline");
            graphTypeCB.addItem("Line");
            graphTypeCB.addItem("Bar");


            groupByLabel.setText("group by");
            graphTypeLabel.setText("Graph type");
            separateByLabel.setText("separate by");
            dataColumnLabel.setText("data");

            Labels_layout.addWidget(&groupByLabel);
            Labels_layout.addWidget(&separateByLabel);
            Labels_layout.addWidget(&dataColumnLabel);
            Labels_layout.addWidget(&graphTypeLabel);

            Labels_layout.addWidget(&showLabelsCheckBox);
            Labels_layout.addWidget(&graphThemeCheckBox);
            SpinBox_layout.addWidget(&saveAsPDFButton);
            SpinBox_layout.addWidget(&copyScriptHandle);

            groupByLabel.setMaximumHeight(16);
            graphTypeLabel.setMaximumHeight(16);
            separateByLabel.setMaximumHeight(16);
            dataColumnLabel.setMaximumHeight(16);

            graph_layout.addLayout(&Labels_layout);
            graph_layout.addLayout(&SpinBox_layout);
            graph_layout.addLayout(&Settings_layout);


            graphThemeCheckBox.setText("White theme");
            showLabelsCheckBox.setText("Labels");
            buildGraphButton.setText("Build graph");
            saveAsPDFButton.setText("Save graph as .png");
            copyScriptHandle.setText("Copy script handle");
            graph_layout.addWidget(&buildGraphButton);
            graph_layout.addWidget(&cv);

            showLabelsCheckBox.setMaximumWidth(55);
            graphThemeCheckBox.setMaximumWidth(90);


            cutfirst.setText("ignore first");
            cutlast.setText("ignore last");

            //cv.setBackgroundBrush(QColor::fromRgb(255,255,255));
            cv.setChart(&chrt);
            //chrt.setBackgroundBrush(QColor::fromRgb(255,255,255));
            chrt.addSeries(&ls);
            cv.setRenderHint(QPainter::Antialiasing,true);

            cv.setBackgroundBrush(QColor::fromRgb(24,24,24));
            cv.setChart(&chrt);
            chrt.setBackgroundBrush(QColor::fromRgb(24,24,24));


            chrt.addAxis(&vax,Qt::AlignmentFlag::AlignBottom);
            chrt.addAxis(&vay,Qt::AlignmentFlag::AlignLeft);

            groupSubstrBegin.setMinimum(-1);
            groupSubstrEnd.setMinimum(-1);

            dataAggMethod.addItem("count");
            dataAggMethod.addItem("sum");
            dataAggMethod.addItem("avg");



            qroupDateYYYYCheckbox.setText("YYYY");
            qroupDateMMCheckbox.setText("MM");
            qroupDateDDCheckbox.setText("DD");
            qroupDatehhCheckbox.setText("hh");
            qroupDatemmCheckbox.setText("mm");
            qroupDatessCheckbox.setText("ss");

            cutfirst.hide();
            cutlast.hide();
            qroupDateYYYYCheckbox.hide();
            qroupDateMMCheckbox.hide();
            qroupDateDDCheckbox.hide();
            qroupDatehhCheckbox.hide();
            qroupDatemmCheckbox.hide();
            qroupDatessCheckbox.hide();
            dataAggMethod.hide();
            groupSubstrEnd.hide();
            groupSubstrBegin.hide();
            graphThemeCheckBox.hide();
            showLabelsCheckBox.hide();
            saveAsPDFButton.hide();
            copyScriptHandle.hide();
            groupBysb.hide();
            separateBysb.hide();
            dataColumnsb.hide();
            groupByLabel.hide();
            separateByLabel.hide();
            dataColumnLabel.hide();
            buildGraphButton.hide();
            cv.hide();
            ls.hide();
            bs.hide();
            graphTypeCB.hide();
            graphTypeLabel.hide();

            bs.setLabelsVisible();

        }

    };
    graph_Window gw;

    NeuralNetwork nn;

    ReplaceWindow* replaceWindow = nullptr;

    void dragEnterEvent(QDragEnterEvent * evt) override;
    void dropEvent(QDropEvent * evt) override;

public slots:
    void updateTabIcons();

    void sendMail(QString host, QString Sender, QString SenderName, QStringList to,QStringList cc, QString Subject, QString messageText, QStringList attachments, QStringList pictutes);

    void splitColumn();
    void addNumeratorColumn();
    void deleteDublicates();
    void levensteinJoin();

    void openDescriptorWindow();

    void updateMisc();

    // On DB switch
    void on_DBNameComboBox_currentTextChanged(const QString &arg1);

    // Connect to db
    void on_ConnectButton_pressed();

    // Run query
    void runSqlAsync();
    void on_pushButton_3_pressed();

    // Query states
    void onQueryBeginCreating();
    void onQueryBegin();
    void onQuerySuccess();
    void UpdateTable();

    // Update label/timer
    void executionTimerTimeout();

    //Autolaunch timer
    void autolaunchCheck();

    // ForceStop query dwonloading
    void on_stopLoadingQueryButton_pressed();
    void on_the500LinesCheckBox_checkStateChanged(const Qt::CheckState &arg1);


    // Data export
    void on_SaveXLSXButton_pressed();
    void on_SaveCSVButton_pressed();
    void on_SaveSQLiteButton_pressed();


    void on_exportDone();


    // Open last saved exel file
    void OpenFile();

    // Open output directory
    void OpenDirectory();

    // Open new app instance
    void OpenNewWindow();


    // Code editor bindings
    void updatesuggestion();
    void FillSuggest();
    void replaceTool();
    void replaceAction();
    void findAction();

    // Copying from tableView
    void CopySelectionFormTable();
    void CopySelectionFormTableSql();

    void tableHeaderDoubleClicked(int id);


    void ImportFromCSV(QString filename, QString delimeter = ";", bool headers = true, bool bypass = false, QString localDBTableName = "tmp");
    void ImportFromXlsx(QString filename,QString Sheetname,bool headers = true);


    void CopyLastLaunchedSql();

    // SubWindow switches
    void ShowHistoryWindow();
    void ShowWorkspacesWindow();
    void ShowTimerWindow();
    void ShowGraph();

    // Graph stuff
    void UpdateGraph();
    void saveGraphAsPDF();
    void copyGraphScriptHandle();
    void saveGraph(DatabaseConnection* tmp_con);

    //Tabs
    //Cycle to next/first tab
    void cycleTabs();
    void on_tabWidget_tabCloseRequested(int index);
    void on_tabWidget_currentChanged(int index);
    void on_tabWidget_tabBarDoubleClicked(int index);

    void on_pushButton_4_clicked();

    //Workspace name change
    void on_workspaceLineEdit_textChanged(QString arg1);

    // History/workspace loading
    void on_TreeItem_Changed(const QItemSelection &selected, const QItemSelection &deselected);
    // Workspace save
    void SaveWorkspace();


    // Pass to right code editor
    void CommentSelected();

    // QML test
    void on_pushButton_pressed();

    // TokenProcessortest
    void on_pushButton_2_pressed();

    //NN tests
    void on_nnTestRun_pressed();
    void on_nnTestLearn_pressed();



private:
    Ui::LoaderWidnow *ui;

signals:
    void TableUpdated();
private slots:
    void on_connection_comboBox_currentTextChanged(const QString &arg1);
    void on_ExportButton_clicked();
    void on_ImportButton_clicked();
};
inline LoaderWidnow* loadWind = nullptr;
#endif // LOADERWIDNOW_H
