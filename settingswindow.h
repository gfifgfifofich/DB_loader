#ifndef SETTINGSWINDOW_H
#define SETTINGSWINDOW_H

#include <QMainWindow>
#include <qcheckbox.h>
#include <qfontcombobox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>

namespace Ui {
class SettingsWindow;
}

class SettingsWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit SettingsWindow(QWidget *parent = nullptr);
    ~SettingsWindow();

    std::vector<QLabel*> varLabels;
    std::vector<QLineEdit*> varLEs;
    std::vector<QPushButton*> varColorButtons;
    std::vector<QFontComboBox*> varFontCBs;
    std::vector<QComboBox*> varCBs;
    std::vector<QCheckBox*> varCheckBoxes;
signals:
    void saved();
private:
    Ui::SettingsWindow *ui;
private slots:
    void cancel();
    void save();
    void cb_pressed(QString whats_this);

};

#endif // SETTINGSWINDOW_H
