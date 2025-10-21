#include "connectionmanager.h"
#include "datastorage.h"
#include "ui_connectionmanager.h"

inline QString usrDir;
inline QString documentsDir;
inline DataStorage userDS;

ConnectionManager::ConnectionManager(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ConnectionManager)
{
    ui->setupUi(this);
    updateList();
}
void ConnectionManager::updateList()
{

    ui->connection_ListWidget->clear();
    for(auto x : userDS.data["Connections"])
        ui->connection_ListWidget->addItem(x.first);

    ui->driver_comboBox->clear();
    QStringList strl;
    for( auto x : userDS.data["UserDrivers"])
    {
        strl.push_back((x.second));
        strl.back() = strl.back().trimmed();
    }
    ui->driver_comboBox->addItems(strl);


    for(int i =0;i < ui->connection_ListWidget->count();i++)
        if(ui->connection_ListWidget->item(i)->text().trimmed() == last_conname.trimmed())
        {
            ui->connection_ListWidget->setCurrentRow(i);
            break;
        }
}

ConnectionManager::~ConnectionManager()
{
    delete ui;
}

void ConnectionManager::on_connection_ListWidget_currentTextChanged(const QString &currentText)
{
    ui->conname_lineEdit->setText(currentText);
    ui->driver_comboBox->setCurrentText(userDS.data[currentText]["Driver"].trimmed());
    ui->constr_lineEdit->setText(userDS.data[currentText]["Connstr"]);
    ui->user_lineEdit->setText(userDS.data[currentText]["User"]);
    ui->password_lineEdit->setText(userDS.data[currentText]["Password"]);
}


void ConnectionManager::on_save_pushButton_clicked()
{

    if(ui->connection_ListWidget->currentItem() == nullptr)
    {
        userDS.Save((documentsDir + "/userdata.txt"));
        return;
    }
    userDS.PopObject(ui->connection_ListWidget->currentItem()->text());
    userDS.PopProperty("Connections",ui->connection_ListWidget->currentItem()->text());


    userDS.data["Connections"][ui->conname_lineEdit->text()] = "";
    userDS.data[ui->conname_lineEdit->text()]["Driver"] = ui->driver_comboBox->currentText();
    userDS.data[ui->conname_lineEdit->text()]["Connstr"] = ui->constr_lineEdit->text();
    userDS.data[ui->conname_lineEdit->text()]["User"] = ui->user_lineEdit->text();
    userDS.data[ui->conname_lineEdit->text()]["Password"] = ui->password_lineEdit->text();

    userDS.Save((documentsDir + "/userdata.txt"));
    last_conname = ui->conname_lineEdit->text();
    updateList();
}


void ConnectionManager::on_cancel_pushButton_clicked()
{
    ui->conname_lineEdit->setText(ui->connection_ListWidget->currentItem()->text());
    ui->driver_comboBox->setCurrentText(userDS.data[ ui->conname_lineEdit->text()]["Driver"]);
    ui->constr_lineEdit->setText(userDS.data[ui->conname_lineEdit->text()]["Connstr"]);
    ui->user_lineEdit->setText(userDS.data[ui->conname_lineEdit->text()]["User"]);
    ui->password_lineEdit->setText(userDS.data[ui->conname_lineEdit->text()]["Password"]);

}


void ConnectionManager::on_new_pushButton_clicked()
{

    ui->conname_lineEdit->setText("NewConnection");
    ui->driver_comboBox->setCurrentText(userDS.data[ui->conname_lineEdit->text()]["Driver"]);
    ui->constr_lineEdit->setText(userDS.data[ui->conname_lineEdit->text()]["Connstr"]);
    ui->user_lineEdit->setText(userDS.data[ui->conname_lineEdit->text()]["User"]);
    ui->password_lineEdit->setText(userDS.data[ui->conname_lineEdit->text()]["Password"]);
    userDS.data["Connections"]["NewConnection"] = "";
    last_conname = ui->conname_lineEdit->text();
    updateList();
}


void ConnectionManager::on_Delete_pushButton_clicked()
{
    userDS.PopObject(ui->connection_ListWidget->currentItem()->text());
    userDS.PopProperty("Connections",ui->connection_ListWidget->currentItem()->text());
    updateList();

}


void ConnectionManager::on_showPassword_checkBox_checkStateChanged(const Qt::CheckState &arg1)
{
    if(arg1 == Qt::Checked)
    {
        ui->password_lineEdit->setEchoMode(QLineEdit::Normal);
    }
    else
    {
        ui->password_lineEdit->setEchoMode(QLineEdit::Password);
    }

}

