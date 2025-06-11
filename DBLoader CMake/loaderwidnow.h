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

#include "NeuralNetwork.h"
#include "codeeditor.h"
#include "databaseconnection.h"


//  Launch options
inline QString launchOpenFileName = "";
inline bool launchOpenFile = false;
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

        QSpinBox groupBysb;
        QSpinBox separateBysb;
        QSpinBox dataColumnsb;

        QComboBox graphTypeCB;
        QLabel graphTypeLabel;

        QLabel groupByLabel;
        QLabel separateByLabel;
        QLabel dataColumnLabel;

        QPushButton buildGraphButton;
        QPushButton saveAsPDFButton;
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

        void Init()
        {
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

            groupByLabel.setMaximumHeight(16);
            graphTypeLabel.setMaximumHeight(16);
            separateByLabel.setMaximumHeight(16);
            dataColumnLabel.setMaximumHeight(16);

            graph_layout.addLayout(&Labels_layout);
            graph_layout.addLayout(&SpinBox_layout);

            graphThemeCheckBox.setText("White theme");
            showLabelsCheckBox.setText("Labels");
            buildGraphButton.setText("Build graph");
            saveAsPDFButton.setText("Save graph as .pdf");
            graph_layout.addWidget(&buildGraphButton);
            graph_layout.addWidget(&cv);

            showLabelsCheckBox.setMaximumWidth(55);
            graphThemeCheckBox.setMaximumWidth(90);

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


            groupBysb.setMinimum(0);
            separateBysb.setMinimum(-1);
            dataColumnsb.setMinimum(0);

            groupBysb.setValue(0);
            separateBysb.setValue(-1);
            dataColumnsb.setValue(0);

            graphThemeCheckBox.hide();
            showLabelsCheckBox.hide();
            saveAsPDFButton.hide();
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


public slots:
    void sendMail(QString host, QString Sender, QString SenderName, QStringList to,QStringList cc, QString Subject, QString messageText, QStringList attachments);


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

    // Data import
    void on_ImportFromCSVButton_pressed();
    void on_importFromExcelButton_pressed();


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

    // Copying from tableView
    void CopySelectionFormTable();
    void CopySelectionFormTableSql();

    void CopyLastLaunchedSql();

    // SubWindow switches
    void ShowHistoryWindow();
    void ShowWorkspacesWindow();
    void ShowTimerWindow();
    void ShowGraph();

    // Graph stuff
    void UpdateGraph();
    void saveGraphAsPDF();
    // Update column labels of graph
    void on_graph_group_change(int val);
    void on_graph_separator_change(int val);
    void on_graph_data_change(int val);

    //Tabs
    //Cycle to next/first tab
    void cycleTabs();
    void on_tabWidget_tabCloseRequested(int index);
    void on_tabWidget_currentChanged(int index);
    void on_tabWidget_tabBarDoubleClicked(int index);

    void on_pushButton_4_clicked();

    //Workspace name change
    void on_workspaceLineEdit_textChanged(const QString &arg1);
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
};
inline LoaderWidnow* loadWind = nullptr;
#endif // LOADERWIDNOW_H
