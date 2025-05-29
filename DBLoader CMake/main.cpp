

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


#include "Shlobj.h"

inline DataStorage userDS;
inline DataStorage historyDS;
inline QString appfilename;
inline QString usrDir;
inline QString documentsDir;

inline QQmlApplicationEngine* TestqmlEngine = nullptr;

int main(int argc, char *argv[])
{
    fillPaterns();
    qDebug()<< "filled patterns";
    QStringList strl = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
    if(strl.size() > 0)
        usrDir = strl[0];

    if(argc <2 && argc > 0)
    {
        usrDir = argv[0];
        if(usrDir.endsWith(".exe"))
        {
            while(!usrDir.endsWith('/') && !usrDir.endsWith('\\') )
            {
                usrDir.resize(usrDir.size()-1);
            }
        }
    }
    strl.clear();

    QStringList strl2 = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation);
    if(strl2.size() > 0)
        documentsDir = strl2[0];


    if(!QDir(documentsDir + "/DBLoader").exists())
        QDir().mkdir(documentsDir + "/DBLoader");


    documentsDir = documentsDir + "/DBLoader";


    //create app file scheme in documents, if not exists already
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
    QDir::setCurrent(usrDir); // set fixed area for .dll's & shit to open .sql files using this app
    qDebug()<< usrDir;
    bool darktheme = false;
    qDebug() << "reached if(userDS.Load((documentsDir + /userdata.txt).toStdString()))" << (documentsDir + "/userdata.txt");
    if(userDS.Load((documentsDir + "/userdata.txt")))
    {
        if(argc >= 2)
        {
            usrDir = userDS.data["user"]["appdir"];
        }


        userDS.data["user"]["appdir"] = usrDir;
    }

    qDebug() << "if(userDS.Load((documentsDir + /userdata.txt)))";

    QApplication::setDesktopSettingsAware(true);

    if(QString(userDS.GetProperty("UserTheme", "DarkMainTheme")).trimmed() == "Dark")
    {
        darktheme = true;
        qputenv("QT_QPA_PLATFORM", "windows:darkmode=2");
        QGuiApplication::styleHints()->setColorScheme(Qt::ColorScheme::Dark);
    }
    else if(QString(userDS.GetProperty("UserTheme", "DarkMainTheme")).trimmed() == "Light")
    {
        darktheme = false;
        qputenv("QT_QPA_PLATFORM", "windows:darkmode=1");
        QGuiApplication::styleHints()->setColorScheme(Qt::ColorScheme::Light);
    }
    qDebug() << "reached app";
    QApplication app(argc, argv);
    app.setStyle("fusion");

    if(argc > 0)
    {
        appfilename = argv[0];
        qDebug()<<"appname is "<< argv[0];
    }
    if(argc>1)
    {
        int i =0;
        while(i < argc)
        {
            if(QString().fromLocal8Bit(argv[i]).endsWith(".exe"))
            {
                i++;
                break;
            }
            else
                i++;
        }
        launchOpenFile = true;
        launchOpenFileName = QString().fromLocal8Bit(argv[i]);


        if(launchOpenFileName.endsWith(".csv") || launchOpenFileName.endsWith(".xlsx") || launchOpenFileName.endsWith(".db") )
        {
            launchOpenFile =false;
            launchOpenFileName = "";
        }
    }


    qDebug() << "QQmlApplicationEngine";
    QQmlApplicationEngine eng;
    TestqmlEngine = &eng;
    QObject::connect(
        TestqmlEngine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { qDebug() << "closing qml subapp "; },
        Qt::QueuedConnection
        );



    qmlRegisterType<TableData>("SourceApplication", 1, 0, "TableData");
    qmlRegisterType<DatabaseConnection>("SourceApplication", 1, 0, "DatabaseConnection");


    qDebug() << "LoaderWidnow w;";
    LoaderWidnow w;

    if(QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark)
    {
        QPalette palette = w.palette();

        // Set the color for a specific role, e.g., background color
        palette.setColor(QPalette::Base, QColor(20, 20, 20)); // Set background to red
        w.setPalette(palette);

    }


    qDebug() << "LoaderWidnow w.showMaximized();";
    w.showMaximized();

    qDebug() << "LoaderWidnow w.showedMaximized();";
    return app.exec();
}




