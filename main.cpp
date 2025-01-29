
#include <QApplication>

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QVariant>
#include <QtCore>
#include <QtGlobal>
#include "Patterns.h"
#include "Table.h"
using namespace std;

inline int thrnum;
int main(int argc, char *argv[])
{
    fillPaterns();
    QApplication a(argc, argv);
    a.setStyle("fusion");

    Table w;
    thrnum++;
    w.conName = QVariant(thrnum).toString();
    w.show();
    return a.exec();

}
