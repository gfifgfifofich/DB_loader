#ifndef LOADERWIDNOW_H
#define LOADERWIDNOW_H

#include <QMainWindow>
#include <qboxlayout.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qthread.h>
#include <qtimer.h>

#include "codeeditor.h"
#include "databaseconnection.h"

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

    /*
     * Create thread
     * Thread creates DB connection
     * thread done -> set dc
     * exec -> set dc back to nullptr
     */
    DatabaseConnection dc;
    QThread* sqlexecThread = nullptr;

    int queryExecutionState = 0;
    QTimer executionTimer;


    bool createconnection = false;
    bool runall = false;

    QString tmpSql = ""; // for history to return back
    QString LastWorkspaceName = "WorkSpace0";

    bool b_showHistoryWindow = false;
    bool b_showWorkspaceWindow = false;
    bool b_showIteratorWindow = true;

    QString autofilename = "";
    bool autosaveXLSX = false;
    bool autosaveSQLITE = false;
    bool autosaveCSV = false;


    QString _replacePrev_What = "";
    QString _replacePrev_With = "";
    int _replacePrev_from = 0;
    int _replacePrev_to = 99999;

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

    void Init(QString conname,QString driver,QString dbname,QString usrname, QString password, QString sql = "", QString SaveFileName = "excel/tmp.xlsx");

private slots:

    void executionTimerTimeout();

    void on_ConnectButton_pressed();

    void runSqlAsync();
    void IterButtonPressed();

    void onQueryBeginCreating();
    void onQueryBegin();
    void onQuerySuccess();
    void UpdateTable();

    void on_SaveXLSXButton_pressed();
    void on_SaveCSVButton_pressed();
    void on_SaveSQLiteButton_pressed();

    void OpenFile();
    void OpenNewWindow();

    void SaveWorkspace();

    void updatesuggestion();
    void FillSuggest();
    void replaceTool();

    void CopySelectionFormTable();
    void CopySelectionFormTableSql();

    void ShowHistoryWindow();
    void ShowWorkspacesWindow();
    void ShowIterateWindow();
    void ShowGraph();

    void on_workspaceLineEdit_textChanged(const QString &arg1);
    void on_listWidget_currentTextChanged(const QString &currentText);
    void on_DBNameComboBox_currentTextChanged(const QString &arg1);

    void on_ImportFromCSVButton_pressed();
    void on_importFromExcelButton_pressed();

    void on_stopLoadingQueryButton_pressed();
    void on_the500LinesCheckBox_checkStateChanged(const Qt::CheckState &arg1);

    void on_pushButton_pressed();

    void on_pushButton_2_pressed();

private:
    Ui::LoaderWidnow *ui;

signals:
    void TableUpdated();
};
#endif // LOADERWIDNOW_H
