

#include <QApplication>
#include <QCoreApplication>
#include <QtQml/qqmlengine.h>
#include <QtQuick/qquickview.h>
#include "Patterns.h"
#include "Table.h"
inline int thrnum;

int main(int argc, char *argv[])
{
    fillPaterns();
    QApplication app(argc, argv);
    app.setStyle("fusion");

    Table w;
    thrnum++;
    w.conName = QVariant(thrnum).toString();
    w.show();



    return app.exec();
}

/*
TODO:
    create separate class for handling tableData (save, import)
    create separate class for handling all DataBase stuff (queries), using tableData
    only then, re-create UI as frontend for tableData & queries
    then re-create UI in QML/JavaScript, to create proper QML/JavaScript interface
Optional:
    add more highlighting features
    add separate keywords per database type (Oracle, Postgres SQlLite)
*/
