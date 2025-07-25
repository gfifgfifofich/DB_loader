#ifndef STRUCTUREDESCRIPTOR_H
#define STRUCTUREDESCRIPTOR_H

#include <QMainWindow>

#include "codeeditor.h"
#include "QListWidgetItem"
namespace Ui {
class StructureDescriptor;
}

class StructureDescriptor : public QMainWindow
{
    Q_OBJECT

public:
    explicit StructureDescriptor(QWidget *parent = nullptr);
    ~StructureDescriptor();
    CodeEditor* cd = nullptr;



    void Init(QString selecteWord);



    struct ChainCell
    {

        QString prevtable = "";
        QString thistable = "";
        QString forceJoinColumn = "";

    };
    QString chainJoinTable = "";
    QVector<ChainCell> chain;


    QString tableStr = "";
    QString columnStr = "";
    void updateTableSearch();

private slots:
    void on_pushButton_clicked();

    void on_lineEdit_2_textChanged(const QString &arg1);

    void on_lineEdit_textChanged(const QString &arg1);

    void on_listWidget_currentTextChanged(const QString &currentText);

    void on_listWidget_2_currentTextChanged(const QString &currentText);

    void on_lineEdit_3_textChanged(const QString &arg1);

    void on_listWidget_2_itemDoubleClicked(QListWidgetItem *item);

    void on_pasteChainButton_pressed();

    void on_clearChainButton_pressed();

    void on_listWidget_itemDoubleClicked(QListWidgetItem *item);

private:
    Ui::StructureDescriptor *ui;
};

#endif // STRUCTUREDESCRIPTOR_H
