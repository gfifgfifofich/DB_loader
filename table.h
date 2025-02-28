#ifndef TABLE_H
#define TABLE_H

#include <QMainWindow>

#include <QSqlQuery>
#include <QSqlResult>
#include <QSqlRecord>
#include "codeeditor.h"
#include <QtCharts/QtCharts>
#include <qcombobox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qmutex.h>
#include <qpushbutton.h>
#include <qthread.h>
#include <QLayout>
#include <QCheckBox>
#include <QSpinBox>

namespace Ui {
class Table;
}

class Table : public QMainWindow
{
    Q_OBJECT

public:
    explicit Table(QWidget *parent = nullptr);
    int sid  = -1, serial  = -1, sql_ID = -1; // probably wont be able to use sqlid

    ~Table();
    void exec();
    QString sqlCode = "";
    QSqlDatabase* dataBase = nullptr;
    QThread* thr = nullptr;
    QThread* thr2 = nullptr;
    void closeEvent(QCloseEvent* ev) override;
    CodeEditor* cd = nullptr;
    QMutex tableDataMutex;
    QVector<std::vector<QVariant>> tableData;
    std::vector<QString> Headers;
    QStringList headers;
    QString conName = "";

    QChart* chrt = new QChart;
    QValueAxis* ax = new QValueAxis;
    QValueAxis* ax2 = new QValueAxis;
    QDateTimeAxis* axdt = new QDateTimeAxis;
    QLineSeries* ls = new QLineSeries;
    QChartView* cv = new QChartView;

    bool bottomAxisIsDate = false;
    bool b_showgraph = false;
    bool b_showhistory = false;
    bool b_showWorkspaces = false;

    bool b_SaveAsWorkSpace = false;

    QStringList workspaceDependenices;
    qint64 LastWorkspaceUpdateInterval = 12*60*60; // 12 hours
    qint64 LastWorkspaceUpdate = 0;
    QString LastWorkspaceName = "WorkSpace0";
    QString tmpFileName = "";
    QString tmpSql = "";

    void runSqlAsync(QString conname,QString driver,QString dbname,QString usrname, QString password, bool createconnection = false, bool runall = false);

    void connectDB(QString conname,QString driver,QString dbname,QString usrname, QString password);
    QString t_conname = "";
    QString t_driver = "";
    QString t_dbname = "";
    QString t_usrname = "";
    QString t_password = "";
    bool reconect_on_exec = false;

    void UpdateTable();

    void RunAsScript(int startfrom = 0);

    QSqlDatabase tdb;

    QString LastDriverName = "";
    QString LastDbName = "";
    bool executing = false;

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

    QString autofilename = "";
    bool autosaveXLSX = false;
    bool autosaveSQLITE = false;
    bool TableUpdatedForGraph = false;
    void SaveToFile();

    iter_Window iw;

    QVBoxLayout* merge_layout = nullptr;

    void Init(QString conname,QString driver,QString dbname,QString usrname, QString password, QString sql = "", QString SaveFileName = "excel/tmp.xlsx");

private slots:
    void on_pushButton_2_clicked();

    void on_pushButton_clicked();

    void on_pushButton_3_clicked();

    void OpenFile();

    void FillSuggest();

    void ThreadReady();

    void updatesuggestion();

    void OpenNewWindow();

    void IterButtonPressed();

    void ShowIterateWindow();

    void on_comboBox_currentTextChanged(const QString &arg1);

    void on_checkBox_checkStateChanged(const Qt::CheckState &arg1);

    void on_SaveToSQLiteTable_clicked();

    void subWindowDone();

    void ShowGraph();
    void UpdateGraphGraph();
    void CopySelectionFormTable();
    void CopySelectionFormTableSql();


    void on_listWidget_currentTextChanged(const QString &currentText);

    void ShowHistoryWindow();

    void ShowWorkspacesWindow();
    void SaveWorkspace();
    void on_lineEdit_4_textChanged(const QString &arg1);


private:
    bool closing = false;
    Ui::Table *ui;
    bool showIteratorWindow = true;


signals:
    void execdReady();
signals:
    void TableUpdated();
};

#endif // TABLE_H
