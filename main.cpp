

#include <QApplication>
#include <QCoreApplication>
#include <QtQml/qqmlengine.h>
#include <QtQuick/qquickview.h>
#include "Patterns.h"
#include "loaderwidnow.h"
#include "table.h"
#include <QQmlApplicationEngine>
#include "databaseconnection.h"
#include "tabledata.h"
inline int thrnum;




inline DataStorage userDS;
inline DataStorage historyDS;
inline QString appfilename;
inline QString usrDir;
inline QString documentsDir;

inline QQmlApplicationEngine* TestqmlEngine = nullptr;

int main(int argc, char *argv[])
{
    fillPaterns();
    usrDir = QStandardPaths::standardLocations(QStandardPaths::HomeLocation)[0];
    documentsDir = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation)[0] ;


    if(!QDir(usrDir+ "/DBLoader").exists())
        QDir().mkdir(usrDir+ "/DBLoader");
    if(!QDir(documentsDir+ "/DBLoader").exists())
        QDir().mkdir(documentsDir+ "/DBLoader");

    //+ "/DBLoader"
    usrDir = usrDir + "/DBLoader";
    documentsDir = documentsDir + "/DBLoader";

    //create app file scheme if not exists

    if(!QDir(documentsDir+ "/CSV").exists())
        QDir().mkdir(documentsDir+ "/CSV");
    if(!QDir(documentsDir+ "/excel").exists())
        QDir().mkdir(documentsDir+ "/excel");
    if(!QDir(documentsDir+ "/FrequencyMaps").exists())
        QDir().mkdir(documentsDir+ "/FrequencyMaps");
    if(!QDir(documentsDir+ "/sqlBackup").exists())
        QDir().mkdir(documentsDir+ "/sqlBackup");
    if(!QDir(documentsDir+ "/workspaces").exists())
        QDir().mkdir(documentsDir+ "/workspaces");

    if(!QFile(documentsDir + "/sqlHistoryList.txt").exists())
    {
        QFile fl(documentsDir + "/sqlHistoryList.txt");
        fl.open(QFile::OpenModeFlag::ReadWrite);
        fl.close();
    }
    if(!QFile(documentsDir + "/userdata.txt").exists())
    {
        QFile fl(documentsDir + "/userdata.txt");
        fl.open(QFile::OpenModeFlag::ReadWrite);
        fl.write(stockUserDataTXT.toLocal8Bit());
        fl.close();
    }

    qDebug()<< usrDir;
    qDebug()<< documentsDir;
    QDir::setCurrent(usrDir); // set fixed area for .dll's &shit
    qDebug()<< usrDir;
    bool darktheme = false;
    if(userDS.Load((documentsDir + "/userdata.txt").toStdString()))
    {
        if(userDS.GetPropertyAsInt("UserTheme", "DarkMainTheme ")>0)
        {
            darktheme = true;
        }
    }

    qputenv("QT_QPA_PLATFORM", "windows:darkmode=1");

    QApplication::setDesktopSettingsAware(false);
    qDebug() << QApplication::desktopSettingsAware();
    QApplication app(argc, argv);
    app.setStyle("fusion");
    if(argc > 0)
    {
        appfilename = argv[0];
        qDebug()<<"appname is "<< argv[0];
    }
    if(argc>1)
    {
        launchOpenFile = true;
        launchOpenFileName = QString().fromLocal8Bit(argv[1]);
        if(launchOpenFileName.endsWith(".csv") ||launchOpenFileName.endsWith(".xlsx") || launchOpenFileName.endsWith(".db") )
        {
            launchOpenFile =false;
            launchOpenFileName = "";
        }
    }
    QQmlApplicationEngine eng;
    TestqmlEngine = &eng;
    QObject::connect(
        TestqmlEngine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection
        );

    // От 0 до 10

    qmlRegisterType<TableData>("SourceApplication", 1, 0, "TableData");
    qmlRegisterType<DatabaseConnection>("SourceApplication", 1, 0, "DatabaseConnection");
    //Table w;
    //thrnum++;
    //w.conName = QVariant(thrnum).toString();
    //w.show();

    LoaderWidnow w;
    //if(!darktheme || true)
    //{
    //    //auto palette = QApplication::palette();
    //    QPalette p;
    //    w.setPalette(p);
    //}
            //thrnum++;
    //w.conName = QVariant(thrnum).toString();

    w.showMaximized();

    w.Init();
    return app.exec();
}

