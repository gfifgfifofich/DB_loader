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
    lt->addWidget(accptbutton);
    lt->addWidget(cancelbutton);
    ui->verticalLayout_2->addLayout(lt);

    connect( accptbutton, SIGNAL(pressed()), this, SLOT(save()), Qt::QueuedConnection );
    connect( cancelbutton, SIGNAL(pressed()), this, SLOT(cancel()), Qt::QueuedConnection );

    // color buttons
    connect(ui->themeColumnNameColorButton,  &QPushButton::pressed, this, [this]()
            {
                cb_pressed(ui->themeColumnNameColorLabel);
                return;
            });
    connect(ui->themeTableNameColorButton,  &QPushButton::pressed, this, [this]()
            {
                cb_pressed(ui->themeTableNameColorLabel);
                return;
            });
    connect(ui->themeCommentColorButton,  &QPushButton::pressed, this, [this]()
            {
                cb_pressed(ui->themeCommentColorLabel);
                return;
            });
    connect(ui->themeQuotationColorButton,  &QPushButton::pressed, this, [this]()
            {
                cb_pressed(ui->themeQuotationColorLabel);
                return;
            });
    connect(ui->themeKeywordColorButton,  &QPushButton::pressed, this, [this]()
            {
                cb_pressed(ui->themeKeywordColorLabel);
                return;
            });

    connect(ui->themeBracketHighlightColorButton,  &QPushButton::pressed, this, [this]()
            {
                cb_pressed(ui->themeBracketHighlightColorLabel);
                return;
            });

    // theme
    ui->themeComboBox->addItem("Light");
    ui->themeComboBox->addItem("Dark");
    ui->themeComboBox->addItem("System");

    ui->themeComboBox->setCurrentText(userDS.data["UserTheme"]["DarkMainTheme"].trimmed());
    ui->themeFontComboBox->setCurrentText(userDS.data["UserTheme"]["Font"].trimmed());
    ui->themeFontSize->setValue(userDS.data["UserTheme"]["FontSize"].trimmed().toInt());


    //languages
    ui->languageComboBox->addItem("en");
    ui->languageComboBox->addItem("ru");
    ui->languageComboBox->setCurrentText(userDS.data["UserTheme"]["Language"].trimmed());

    // local database conection
    // for(auto x : userDS.data["UserDrivers"])
    // {
    //     ui->LocalDBDriverComboBox->addItem(x.second.trimmed());
    // }
    // ui->LocalDBDriverComboBox->setCurrentText(userDS.data["UserTheme"]["db_drv_Save_table_driver"].trimmed());
    //userDS.data["UserTheme"]["db_drv_Save_table_Connection"] = ui->LocalDBConnection_ComboBox->currentText().trimmed();

    ui->LocalDBConnection_ComboBox->clear();
    QStringList strl;
    for(auto x : userDS.data["Connections"])
        strl.push_back(x.first);
    ui->LocalDBConnection_ComboBox->addItems(strl);

    ui->LocalDBConnection_ComboBox->setCurrentText(userDS.data["UserTheme"]["db_drv_Save_table_Connection"].trimmed());


    //Code preview scales
    ui->codePreviewLineScale->setValue(userDS.data["UserTheme"]["codePreviewLineScale"].toDouble());
    ui->codePreviewWidth->setValue(userDS.data["UserTheme"]["codePreviewWidth"].toDouble());

    // some bools
    ui->codeSuggestionCheckBox->setChecked(userDS.data["UserTheme"]["ShowCodeSuggestion"].trimmed() == "true");
    ui->codePreviewCheckBox->setChecked(userDS.data["UserTheme"]["CodePreview"].trimmed() == "true");
    ui->codePreviewAntialiasingCheckBox->setChecked(userDS.data["UserTheme"]["CodePreviewAntialiasing"].trimmed() == "true");



    //user theme bolds and italics
    ui->themeKeywordBoldCheckBox->setChecked(userDS.data["UserTheme"]["Bold_KeyWord"].trimmed() == "true");
    ui->themeColumnNameBoldCheckBox->setChecked(userDS.data["UserTheme"]["Bold_column_name"].trimmed() == "true");
    ui->themeTableNameBoldCheckBox->setChecked(userDS.data["UserTheme"]["Bold_table_name"].trimmed() == "true");
    ui->themeCommentBoldCheckBox->setChecked(userDS.data["UserTheme"]["Bold_comment"].trimmed() == "true");
    ui->themeQuotationBoldCheckBox->setChecked(userDS.data["UserTheme"]["Bold_quotation"].trimmed() == "true");

    ui->themeKeywordItalicCheckBox->setChecked(userDS.data["UserTheme"]["Italic_KeyWord"].trimmed() == "true");
    ui->themeColumnNameItalicCheckBox->setChecked(userDS.data["UserTheme"]["Italic_column_name"].trimmed() == "true");
    ui->themeTableNameItalicCheckBox->setChecked(userDS.data["UserTheme"]["Italic_table_name"].trimmed() == "true");
    ui->themeCommentItalicCheckBox->setChecked(userDS.data["UserTheme"]["Italic_comment"].trimmed() == "true");
    ui->themeQuotationItalicCheckBox->setChecked(userDS.data["UserTheme"]["Italic_quotation"].trimmed() == "true");


    // user theme colors

    QLabel* trg = ui->themeColumnNameColorLabel;
    trg->setText(userDS.data["UserTheme"]["Color_column_name"].trimmed());

    strl = trg->text().split(',');
    QColor col = QColor::fromRgbF(0.25f,0.55f,0.95f,1.0f);
    for(int i=0;i<strl.size();i++)
    {
        if(i==0) col.setRed(QVariant(strl[i]).toInt());
        if(i==1) col.setGreen(QVariant(strl[i]).toInt());
        if(i==2) col.setBlue(QVariant(strl[i]).toInt());
        if(i==3) col.setAlpha(QVariant(strl[i]).toInt());
    }
    QPalette palette = trg->palette();
    palette.setColor(trg->foregroundRole(), col);
    trg->setPalette(palette);



    trg = ui->themeTableNameColorLabel;
    trg->setText(userDS.data["UserTheme"]["Color_table_name"].trimmed());

    strl = trg->text().split(',');
    col = QColor::fromRgbF(0.25f,0.55f,0.95f,1.0f);
    for(int i=0;i<strl.size();i++)
    {
        if(i==0) col.setRed(QVariant(strl[i]).toInt());
        if(i==1) col.setGreen(QVariant(strl[i]).toInt());
        if(i==2) col.setBlue(QVariant(strl[i]).toInt());
        if(i==3) col.setAlpha(QVariant(strl[i]).toInt());
    }
    palette = trg->palette();
    palette.setColor(trg->foregroundRole(), col);
    trg->setPalette(palette);



    trg = ui->themeCommentColorLabel;
    trg->setText(userDS.data["UserTheme"]["Color_comment"].trimmed());

    strl = trg->text().split(',');
    col = QColor::fromRgbF(0.25f,0.55f,0.95f,1.0f);
    for(int i=0;i<strl.size();i++)
    {
        if(i==0) col.setRed(QVariant(strl[i]).toInt());
        if(i==1) col.setGreen(QVariant(strl[i]).toInt());
        if(i==2) col.setBlue(QVariant(strl[i]).toInt());
        if(i==3) col.setAlpha(QVariant(strl[i]).toInt());
    }
    palette = trg->palette();
    palette.setColor(trg->foregroundRole(), col);
    trg->setPalette(palette);



    trg = ui->themeQuotationColorLabel;
    trg->setText(userDS.data["UserTheme"]["Color_quotation"].trimmed());

    strl = trg->text().split(',');
    col = QColor::fromRgbF(0.25f,0.55f,0.95f,1.0f);
    for(int i=0;i<strl.size();i++)
    {
        if(i==0) col.setRed(QVariant(strl[i]).toInt());
        if(i==1) col.setGreen(QVariant(strl[i]).toInt());
        if(i==2) col.setBlue(QVariant(strl[i]).toInt());
        if(i==3) col.setAlpha(QVariant(strl[i]).toInt());
    }
    palette = trg->palette();
    palette.setColor(trg->foregroundRole(), col);
    trg->setPalette(palette);



    trg = ui->themeKeywordColorLabel;
    trg->setText(userDS.data["UserTheme"]["Color_KeyWord"].trimmed());

    strl = trg->text().split(',');
    col = QColor::fromRgbF(0.25f,0.55f,0.95f,1.0f);
    for(int i=0;i<strl.size();i++)
    {
        if(i==0) col.setRed(QVariant(strl[i]).toInt());
        if(i==1) col.setGreen(QVariant(strl[i]).toInt());
        if(i==2) col.setBlue(QVariant(strl[i]).toInt());
        if(i==3) col.setAlpha(QVariant(strl[i]).toInt());
    }
    palette = trg->palette();
    palette.setColor(trg->foregroundRole(), col);
    trg->setPalette(palette);



    trg = ui->themeBracketHighlightColorLabel;
    trg->setText(userDS.data["UserTheme"]["Color_BracketHighlight"].trimmed());

    strl = trg->text().split(',');
    col = QColor::fromRgbF(0.25f,0.55f,0.95f,1.0f);
    for(int i=0;i<strl.size();i++)
    {
        if(i==0) col.setRed(QVariant(strl[i]).toInt());
        if(i==1) col.setGreen(QVariant(strl[i]).toInt());
        if(i==2) col.setBlue(QVariant(strl[i]).toInt());
        if(i==3) col.setAlpha(QVariant(strl[i]).toInt());
    }
    palette = trg->palette();
    palette.setColor(trg->foregroundRole(), col);
    trg->setPalette(palette);
}

SettingsWindow::~SettingsWindow()
{
    delete ui;
}

void SettingsWindow::cb_pressed(QLabel* trg)
{
    QStringList strl = trg->text().split(',');
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
        return;
    trg->setText(
        QVariant(newColor.red()).toString() + "," +
        QVariant(newColor.green()).toString() + "," +
        QVariant(newColor.blue()).toString() + "," +
        QVariant(newColor.alpha()).toString());

    QPalette palette = trg->palette();
    palette.setColor(trg->foregroundRole(), newColor);
    trg->setPalette(palette);
    return;
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
    userDS.Load((documentsDir + "/userdata.txt"));



    userDS.data["UserTheme"]["DarkMainTheme"] = ui->themeComboBox->currentText();
    userDS.data["UserTheme"]["Font"] = ui->themeFontComboBox->currentText();
    userDS.data["UserTheme"]["FontSize"] = QVariant(ui->themeFontSize->value()).toString();


    //languages
    userDS.data["UserTheme"]["Language"] = ui->languageComboBox->currentText();

    // local database conection
    userDS.data["UserTheme"]["db_drv_Save_table_Connection"] = ui->LocalDBConnection_ComboBox->currentText().trimmed();


    //Code preview scales
    userDS.data["UserTheme"]["codePreviewLineScale"] = QVariant(ui->codePreviewLineScale->value()).toString();
    userDS.data["UserTheme"]["codePreviewWidth"] = QVariant(ui->codePreviewWidth->value()).toString();

    // some bools
    userDS.data["UserTheme"]["ShowCodeSuggestion"] = ui->codeSuggestionCheckBox->isChecked() ? "true" : "false";
    userDS.data["UserTheme"]["CodePreview"] = ui->codePreviewCheckBox->isChecked() ? "true" : "false";
    userDS.data["UserTheme"]["CodePreviewAntialiasing"] = ui->codePreviewAntialiasingCheckBox->isChecked() ? "true" : "false";




    userDS.data["UserTheme"]["Bold_KeyWord"    ] = ui->themeKeywordBoldCheckBox   ->isChecked() ? "true" : "false";
    userDS.data["UserTheme"]["Bold_column_name"] = ui->themeColumnNameBoldCheckBox->isChecked() ? "true" : "false";
    userDS.data["UserTheme"]["Bold_table_name" ] = ui->themeTableNameBoldCheckBox ->isChecked() ? "true" : "false";
    userDS.data["UserTheme"]["Bold_comment"    ] = ui->themeCommentBoldCheckBox   ->isChecked() ? "true" : "false";
    userDS.data["UserTheme"]["Bold_quotation"  ] = ui->themeQuotationBoldCheckBox ->isChecked() ? "true" : "false";

    userDS.data["UserTheme"]["Italic_KeyWord"    ] = ui->themeKeywordItalicCheckBox   ->isChecked() ? "true" : "false";
    userDS.data["UserTheme"]["Italic_column_name"] = ui->themeColumnNameItalicCheckBox->isChecked() ? "true" : "false";
    userDS.data["UserTheme"]["Italic_table_name" ] = ui->themeTableNameItalicCheckBox ->isChecked() ? "true" : "false";
    userDS.data["UserTheme"]["Italic_comment"    ] = ui->themeCommentItalicCheckBox   ->isChecked() ? "true" : "false";
    userDS.data["UserTheme"]["Italic_quotation"  ] = ui->themeQuotationItalicCheckBox ->isChecked() ? "true" : "false";






    // user theme colors

    QLabel* trg = ui->themeColumnNameColorLabel;
    userDS.data["UserTheme"]["Color_column_name"] = trg->text();


    trg = ui->themeTableNameColorLabel;
    userDS.data["UserTheme"]["Color_table_name"] = trg->text();

    trg = ui->themeCommentColorLabel;
    userDS.data["UserTheme"]["Color_comment"] = trg->text();

    trg = ui->themeQuotationColorLabel;
    userDS.data["UserTheme"]["Color_quotation"] = trg->text();


    trg = ui->themeKeywordColorLabel;
    userDS.data["UserTheme"]["Color_KeyWord"] = trg->text();


    trg = ui->themeBracketHighlightColorLabel;
    userDS.data["UserTheme"]["Color_BracketHighlight"] = trg->text();





    userDS.Save((documentsDir + "/userdata.txt"));
    emit saved();
    close();
}
