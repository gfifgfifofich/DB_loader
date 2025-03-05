#include "replacedialog.h"
#include "ui_replacedialog.h"


replaceDialog::replaceDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::replaceDialog)
{
    ui->setupUi(this);
    this->setObjectName("replace tool");
}

replaceDialog::~replaceDialog()
{
    delete ui;
}

void replaceDialog::Init()
{
    ui->spinBox->setValue(start_line);
    ui->spinBox_2->setValue(end_line);
    ui->lineEdit->setText(replaceWhat);
    ui->lineEdit_2->setText(replaceWith);
}

void replaceDialog::on_spinBox_valueChanged(int arg1)
{//from
    start_line = arg1;
}

void replaceDialog::on_spinBox_2_valueChanged(int arg1)
{//to
    end_line = arg1;
}

void replaceDialog::on_lineEdit_textChanged(const QString &arg1)
{//replace what
    replaceWhat = arg1;
}


void replaceDialog::on_lineEdit_2_textChanged(const QString &arg1)
{// replace with
    replaceWith = arg1;
}

