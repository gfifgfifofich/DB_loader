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
#include <qlabel.h>
#include <qlineedit.h>
#include <qlineseries.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qsplineseries.h>
#include <qthread.h>
#include <qtimer.h>
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

    CodeEditor* cd = nullptr;

    DatabaseConnection dc;
    QThread* sqlexecThread = nullptr;

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

    // init used inly in scripts
    void Init();
    //autosave after exec, used only in depricated scripts
    QString autofilename = "";
    bool autosaveXLSX = false;
    bool autosaveSQLITE = false;
    bool autosaveCSV = false;

    // used inly in scripts
    bool createconnection = false;
    bool runall = false;

    // depracated
    struct iter_Window
    {
        QVBoxLayout iter_layout;
        QHBoxLayout SpinBox_layout;
        QHBoxLayout Labels_layout;

        QSpinBox sb1;
        QSpinBox sb2;
        QSpinBox sb3;

        QLabel lbls1;
        QLabel lbls2;
        QLabel lbls3;

        QLineEdit nameline;

        QPushButton button;


        void Init()
        {
            SpinBox_layout.addWidget(&sb1);
            SpinBox_layout.addWidget(&sb2);
            SpinBox_layout.addWidget(&sb3);

            lbls1.setText("first");
            lbls2.setText("last");
            lbls3.setText("step");

            Labels_layout.addWidget(&lbls1);
            Labels_layout.addWidget(&lbls2);
            Labels_layout.addWidget(&lbls3);
            lbls1.setMaximumHeight(16);
            lbls2.setMaximumHeight(16);
            lbls3.setMaximumHeight(16);

            iter_layout.addLayout(&Labels_layout);
            iter_layout.addLayout(&SpinBox_layout);

            nameline.setPlaceholderText("Name_To_Replace");
            iter_layout.addWidget(&nameline);

            button.setText("Run in separate windows");
            iter_layout.addWidget(&button);

            sb1.hide();
            sb2.hide();
            sb3.hide();
            lbls1.hide();
            lbls2.hide();
            lbls3.hide();
            nameline.hide();
            button.hide();
        }

    };
    iter_Window iw;


    NeuralNetwork nn;

private slots:

    // on DB switch
    void on_DBNameComboBox_currentTextChanged(const QString &arg1);

    // connect to db
    void on_ConnectButton_pressed();

    // run query
    void runSqlAsync();
    void on_pushButton_3_pressed();

    // Query states
    void onQueryBeginCreating();
    void onQueryBegin();
    void onQuerySuccess();
    void UpdateTable();

    // update label/timer
    void executionTimerTimeout();


    // ForceStop query dwonloading
    void on_stopLoadingQueryButton_pressed();
    void on_the500LinesCheckBox_checkStateChanged(const Qt::CheckState &arg1);


    // data export
    void on_SaveXLSXButton_pressed();
    void on_SaveCSVButton_pressed();
    void on_SaveSQLiteButton_pressed();

    // data import
    void on_ImportFromCSVButton_pressed();
    void on_importFromExcelButton_pressed();


    // open last saved exel file
    void OpenFile();

    // open new app instance
    void OpenNewWindow();



    // Code editor bindings
    void updatesuggestion();
    void FillSuggest();
    void replaceTool();

    // Copying from tableView
    void CopySelectionFormTable();
    void CopySelectionFormTableSql();

    // SubWindow switches
    void ShowHistoryWindow();
    void ShowWorkspacesWindow();
    void ShowIterateWindow();
    void ShowGraph();

    // Graph stuff
    void UpdateGraph();
    void saveGraphAsPDF();
    // Update column labels of graph
    void on_graph_group_change(int val);
    void on_graph_separator_change(int val);
    void on_graph_data_change(int val);


    //Workspace name change
    void on_workspaceLineEdit_textChanged(const QString &arg1);
    // history/workspace loading
    void on_listWidget_currentTextChanged(const QString &currentText);

    void SaveWorkspace();



    // QML test
    void on_pushButton_pressed();
    // TokenProcessortest
    void on_pushButton_2_pressed();
    //NN tests
    void on_nnTestRun_pressed();
    void on_nnTestLearn_pressed();




    //Depricated
    void IterButtonPressed();


private:
    Ui::LoaderWidnow *ui;

signals:
    void TableUpdated();
};
#endif // LOADERWIDNOW_H
