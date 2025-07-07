#ifndef DOCWINDOW_H
#define DOCWINDOW_H

#include <QMainWindow>
#include <qlabel.h>
#include <qtreewidget.h>
#include "codeeditor.h"

namespace Ui {
class DocWindow;
}

class DocWindow : public QMainWindow
{
    Q_OBJECT

public:

    explicit DocWindow(QWidget *parent = nullptr);
    ~DocWindow();

private slots:
    void on_treeWidget_itemChanged(QTreeWidgetItem *item, int column);

    void on_treeWidget_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);

private:
    Ui::DocWindow *ui;

    std::vector<CodeEditor*> cd_ptrs;
    std::vector<QLabel*> label_ptrs;

};

#endif // DOCWINDOW_H
