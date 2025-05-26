#ifndef DATABASETASK_H
#define DATABASETASK_H

#include "databaseconnection.h"
#include <QObject>
#include <QQmlEngine>

class DatabaseTask : public QObject
{
    Q_OBJECT
    QML_ELEMENT
public:
    explicit DatabaseTask(QObject *parent = nullptr);

    DatabaseConnection dc;
    QThread* sqlexecThread = nullptr;
    QString autofilename = "";
    bool autosaveXLSX = false;
    bool autosaveSQLITE = false;
    bool autosaveCSV = false;

    QString lastError = "";
private slots:
    void exec();

signals:
    void executed();
    void saved();
};

#endif // DATABASETASK_H
