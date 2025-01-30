#ifndef TABLE_H
#define TABLE_H

#include <QMainWindow>

#include <QSqlQuery>
#include <QSqlResult>
#include <QSqlRecord>
#include "codeeditor.h"
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
    ~Table();
    void exec();
    QString sqlCode = "";
    QSqlDatabase* dataBase = nullptr;
    QThread* thr = nullptr;
    void closeEvent(QCloseEvent* ev) override;
    CodeEditor* cd = nullptr;

    QMutex tableDataMutex;
    std::vector<std::vector<QVariant>> tableData;
    std::vector<QString> Headers;
    QStringList headers;
    QString conName = "";
    void runSqlAsync(QString conname,QString dbname,QString usrname, QString password, bool createconnection = false, bool runall = false);

    void connectDB(QString conname,QString dbname,QString usrname, QString password);

    void UpdateTable();
    QSqlDatabase tdb;

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
    bool autosave = false;
    void SaveToFile();

    iter_Window iw;

    QVBoxLayout* merge_layout = nullptr;

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

private:
    bool closing = false;
    Ui::Table *ui;
    bool showIteratorWindow = true;


signals:
    void execdReady();
};

#endif // TABLE_H

