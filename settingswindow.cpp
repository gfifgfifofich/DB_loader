#include "settingswindow.h"
#include "datastorage.h"
#include "ui_settingswindow.h"
#include <qcolordialog.h>
#include <qdatetime.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include "colorbutton.h"
#include <QInputDialog>

inline QString usrDir;
inline QString documentsDir;
inline DataStorage userDS;
inline DataStorage historyDS;
inline QString appfilename;

SettingsWindow::SettingsWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::SettingsWindow)
{
    ui->setupUi(this);

    QPushButton* accptbutton = new QPushButton("Save",this);
    QPushButton* cancelbutton = new QPushButton("Cancel",this);
    QHBoxLayout* lt = new QHBoxLayout();

    connect( accptbutton, SIGNAL(pressed()), this, SLOT(save()), Qt::QueuedConnection );
    connect( cancelbutton, SIGNAL(pressed()), this, SLOT(cancel()), Qt::QueuedConnection );

    lt->addWidget(accptbutton);
    lt->addWidget(cancelbutton);
    //QColor newColor = QColorDialog::getColor();


    for(auto x : userDS.data["UserTheme"])
    {
        QString var = x.first.c_str();

        QLabel* varlbl = new QLabel(var);
        QLineEdit* varLE = new QLineEdit(x.second.c_str());
        ColorButton* varColorbutton = new ColorButton("pick color");
        QFontComboBox* varFontCB = new QFontComboBox();
        QComboBox* varCB = new QComboBox();
        QCheckBox* varCheckBox = new QCheckBox();

        QHBoxLayout* valLt = new QHBoxLayout();

        varlbl->setWhatsThis(var);
        varLE->setWhatsThis(var);
        varColorbutton->setWhatsThis(var);

        valLt->addWidget(varlbl);
        if(var.startsWith("Color"))
        {
            // add color select button
            valLt->addWidget(varLE);
            valLt->addWidget(varColorbutton);
            connect( varColorbutton, SIGNAL(pressed()), varColorbutton, SLOT(on_pressed()));
            connect( varColorbutton, SIGNAL(cb_pressed(QString)), this, SLOT(cb_pressed(QString)));
        }
        else if(var == "Font")
        {
            // add color select button

            varFontCB->setCurrentFont(QFont(QString(x.second.c_str()).trimmed()));
            valLt->addWidget(varFontCB);
        }
        else if(var.startsWith("Bold") || var.startsWith("Italic") || var == "ShowTestButtons" || var == "CodePreview"|| var == "CodePreviewAntialiasing" )
        {
            // add color select button
            if(varLE->text().trimmed() == "true")
                varCheckBox->setCheckState(Qt::Checked);
            else
                varCheckBox->setCheckState(Qt::Unchecked);

            valLt->addWidget(varCheckBox);
        }
        else if(var == "DarkMainTheme")
        {

            // add color select button
            varCB->addItem("Light");
            varCB->addItem("Dark");
            varCB->addItem("System");

            varCB->setCurrentText(varLE->text().trimmed());

            valLt->addWidget(varCB);
        }
        else
            valLt->addWidget(varLE);

        varLabels.push_back(varlbl);
        varLEs.push_back(varLE);
        varColorButtons.push_back(varColorbutton);
        varFontCBs.push_back(varFontCB);
        varCBs.push_back(varCB);
        varCheckBoxes.push_back(varCheckBox);

        ui->verticalLayout->addLayout(valLt);
    }

    ui->verticalLayout->addLayout(lt);
}

SettingsWindow::~SettingsWindow()
{
    delete ui;
}

void SettingsWindow::cb_pressed(QString whats_this)
{

    for(auto x : varLEs)
    {
        if(x->whatsThis() == whats_this)
        {
            QStringList strl = x->text().split(',');
            QColor col = QColor::fromRgbF(0.25f,0.55f,0.95f,1.0f);
            for(int i=0;i<strl.size();i++)
            {
                if(i==0) col.setRed(QVariant(strl[i]).toInt());
                if(i==1) col.setGreen(QVariant(strl[i]).toInt());
                if(i==2) col.setBlue(QVariant(strl[i]).toInt());
                if(i==3) col.setAlpha(QVariant(strl[i]).toInt());
            }


            QColor newColor = QColorDialog::getColor(col);
            if(newColor.red() ==0 && newColor.green() ==0 && newColor.blue() ==0 && newColor.alpha() == 255)
                break;
            x->setText(
                QVariant(newColor.red()).toString() + "," +
                    QVariant(newColor.green()).toString() + "," +
                    QVariant(newColor.blue()).toString() + "," +
                QVariant(newColor.alpha()).toString());
            break;
        }
    }

//    BracketHighlightColor 5,5,35,255;
}

void SettingsWindow::cancel()
{
    close();
}
void SettingsWindow::save()
{
    for(int i=0;i < varLEs.size();i++)
    {

        if(varLEs[i]->whatsThis() == "Font")
        {
            varLEs[i]->setText(varFontCBs[i]->currentText());
        }
        else if(varLEs[i]->whatsThis().startsWith("Bold") || varLEs[i]->whatsThis().startsWith("Italic") || varLEs[i]->whatsThis() == "ShowTestButtons" || varLEs[i]->whatsThis() == "CodePreview"|| varLEs[i]->whatsThis() == "CodePreviewAntialiasing" )
        {
            if(varCheckBoxes[i]->isChecked())
                varLEs[i]->setText("true");
            else
                varLEs[i]->setText("false");
        }
        else if(varLEs[i]->whatsThis() == "DarkMainTheme")
        {
            varLEs[i]->setText(varCBs[i]->currentText());
        }

        userDS.SetProperty("UserTheme",varLEs[i]->whatsThis().toStdString(),varLEs[i]->text().toStdString());

    }
    userDS.Save((documentsDir + "/userdata.txt").toStdString());
    emit saved();
    close();
}
