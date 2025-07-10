

#include <QApplication>
#include <QCoreApplication>
#include "Patterns.h"
#include "loaderwidnow.h"
#include "table.h"
#include <QTranslator>

inline DataStorage userDS;
inline QString appfilename;
inline QString usrDir;
inline QString documentsDir;


int main(int argc, char *argv[])
{


    // fill dictionaries of database-specific keywords
    fillPaterns();


    // get location of "Home"
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
    // this didnt fix crash in debug mode. Deleting any QStringList on windows while in debug mode will cause crash.
    strl.clear();


    // get location of "Documents"
    strl = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation);
    if(strl.size() > 0)
        documentsDir = strl[0];

    //set documents location to be in special for this program directory
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

    if(!QFile(documentsDir + "/userdata.txt").exists())
    {
        QFile fl(documentsDir + "/userdata.txt");
        fl.open(QFile::OpenModeFlag::ReadWrite);
        fl.write(stockUserDataTXT.toLocal8Bit());
        fl.close();
    }

    // show directories in QDebug, just in case
    qDebug()<< usrDir;
    qDebug()<< documentsDir;
    QDir::setCurrent(usrDir); // set fixed area for .dll's & shit to open .sql files using this app
    qDebug()<< usrDir;

    // load user theme prepass, before app launch
    bool darktheme = false;
    if(userDS.Load((documentsDir + "/userdata.txt")))
    {
        // check if launched with file to be opened, if so - set application directory to load all the .dll's from that directory. Otherwise app will not be able to do anything
        if(argc >= 2)
        {
            usrDir = userDS.data["User"]["appdir"];
        }

        userDS.data["User"]["appdir"] = usrDir;
        if(userDS.data["UserTheme"]["Language"].trimmed().size()<=1)
        {
            userDS.data["UserTheme"]["Language"] = "en";
        }
        userDS.Save((documentsDir + "/userdata.txt"));
    }


    // set app to be aware about user color scheme, (light/dark). Force one or another, depending on settings
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

    // set style
    QApplication app(argc, argv);
    app.setStyle("fusion");


    // get app file name, and files,passed through command line
    if(argc > 0)
        appfilename = argv[0];
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





    // choose language
    QTranslator qtTranslator;
    if(userDS.data["UserTheme"]["Language"].trimmed() == "ru")
    if(qtTranslator.load("untitled_ru.qm"))
    {
        qDebug() << "loaded translation";
        app.installTranslator(&qtTranslator);
        qDebug() << qtTranslator.language();
    }

    LoaderWidnow w;

    //if dark theme, need to change some stuff from stock, to be more readable
    if(QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark)
    {
        QPalette palette = w.palette();
        // Set the color for a specific role, e.g., background color
        palette.setColor(QPalette::Base, QColor(20, 20, 20)); // Set background to red
        w.setPalette(palette);
    }

    // launch in full screen
    w.showMaximized();



    return app.exec();
}




