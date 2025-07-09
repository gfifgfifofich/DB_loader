#include "replacewindow.h"
#include "ui_replacewindow.h"
#include "loaderwidnow.h"

ReplaceWindow::ReplaceWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ReplaceWindow)
{
    ui->setupUi(this);
}

ReplaceWindow::~ReplaceWindow()
{
    delete ui;
}

void ReplaceWindow::Init()
{
    ui->spinBox->setValue(start_line);
    ui->spinBox_2->setValue(end_line);
    ui->lineEdit->setText(replaceWhat);
    ui->lineEdit_2->setText(replaceWith);
}
void ReplaceWindow::on_lineEdit_textChanged(const QString &arg1)
{//replace what

    replaceWhat = arg1;
}


void ReplaceWindow::on_lineEdit_2_textChanged(const QString &arg1)
{// replace with
    if(!ui->checkBox->isChecked())
        replaceWith = arg1;

}


void ReplaceWindow::on_spinBox_valueChanged(int arg1)
{//from

    start_line = arg1;
}


void ReplaceWindow::on_spinBox_2_valueChanged(int arg1)
{//to

    end_line = arg1;
}


void ReplaceWindow::on_checkBox_checkStateChanged(const Qt::CheckState &arg1)
{

    if(arg1 == Qt::Checked)
        replaceWith = '\n';
}


void ReplaceWindow::on_pushButton_clicked()
{// find
    loadWind->replaceWindow = this;
    loadWind->findAction();
}


void ReplaceWindow::on_pushButton_2_clicked()
{// replace
    loadWind->replaceWindow = this;
    loadWind->replaceAction();
}

