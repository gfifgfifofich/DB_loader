#include "datastorage.h"
#include <fstream>
#include <iostream>
#include <qdir.h>
#include <sstream>
#include <qdebug.h>
#include <qlogging.h>

/*
yoinked from engine
*/

void DataStorage::AddObject(QString ObjectName)
{
    std::map<QString, QString> a;
    data.insert(std::pair<QString, std::map<QString, QString>>(ObjectName, a));
}


void DataStorage::AddProperty(QString ObjectName, QString Property, QString value)
{
    data[ObjectName].insert(std::pair < QString, QString>(Property, value));
}


void DataStorage::PopProperty(QString ObjectName, QString Property)
{
    data[ObjectName].erase(Property);

}
void DataStorage::PopObject(QString ObjectName)
{
    data.erase(ObjectName);
}


std::map<QString, QString> DataStorage::GetObject(QString ObjectName)
{
    return data[ObjectName];
}

std::vector<QString> DataStorage::GetProperties(QString ObjectName)
{
    std::vector<QString> Properties;
    for (auto i : data[ObjectName])
        Properties.push_back(i.first);

    return Properties;
}

QString DataStorage::GetProperty(QString ObjectName, QString Property)
{
    QString prop = "";
    //if(data.count(ObjectName) > 0 && data[ObjectName].count(Property) > 0)
        prop = data[ObjectName][Property];
    return prop.trimmed();
}

int DataStorage::GetPropertyAsInt(QString ObjectName, QString Property)
{
    //if(data.count(ObjectName) > 0 && data[ObjectName].count(Property) > 0)
        return QVariant(data[ObjectName][Property]).toInt();
    //else return 0;
}
float DataStorage::GetPropertyAsFloat(QString ObjectName, QString Property)
{
    ///if(data.count(ObjectName) > 0 && data[ObjectName].count(Property) > 0)
        return QVariant(data[ObjectName][Property]).toFloat();
    //else return 0;
}
bool DataStorage::GetPropertyAsBool(QString ObjectName, QString Property)
{
    //if(data.count(ObjectName) > 0 && data[ObjectName].count(Property) > 0)
        return QVariant(data[ObjectName][Property]).toBool();
    //else return 0;
}

void DataStorage::SetProperty(QString ObjectName, QString Property, QString value)
{
    data[ObjectName][Property] = value;
}

void DataStorage::SetProperty(QString ObjectName,QString Property, bool value)
{
    data[ObjectName][Property] = QVariant(value).toString();

}
void DataStorage::SetProperty(QString ObjectName,QString Property, int value)
{
    data[ObjectName][Property] = QVariant(value).toString();

}
void DataStorage::SetProperty(QString ObjectName,QString Property, float value)
{
    data[ObjectName][Property] = QVariant(value).toString();
}


void DataStorage::Save(QString filename)
{


    QFile File(filename);
    File.remove();

    if(!File.open(QFile::OpenModeFlag::ReadWrite))
    {
        qDebug() << "Failed to write";
        return;
    }
    for (auto i : data)
    {
        //qDebug() << "   " << i.first;
        File.write(i.first.toUtf8().constData());
        //qDebug() << "   written" << i.first;
        File.write("\n{");

        for (auto p : i.second)
        {
            File.write("\n	");
            File.write(p.first.toUtf8().constData());
            File.write(" ");
            File.write(p.second.toUtf8().constData());
            File.write("");
        }

        File.write("\n}\n");
    }
    //File.close();
}

QString DataStorage::ToString()
{
    QString File = "";

    for (auto i : data)
    {
        File += i.first;
        File += "\n{";

        for (auto p : i.second)
        {
            File += "\n	";
            File += p.first;
            File += " ";
            File += p.second;
            File += "";
        }

        File += "\n}\n";
    }
    return File;
}

bool DataStorage::Load(QString filename)
{
    for(auto d :data)
        d.second.clear();
    data.clear();
    QFile f(filename);
    if (!f.open(QFile::OpenModeFlag::ReadOnly))
    {
        qDebug() << "ERROR LOADING SaveFile: Unable to load " << filename;
        return false;
    }
    QString lastObject = "NULL";
    int lineCount = 0;
    char junk;
    char line[10256];
    while (!f.atEnd())
    {
        lineCount++;
        //qDebug() << "ds rolling "<< lineCount;

        int read_size = f.readLine(line,10256);
        //qDebug() << line;

        std::stringstream s;
        s << line;
        if (line[0] == '{')
            continue;
        if (line[0] == '}' && lastObject !="NULL")
        {
            lastObject = "NULL";
            continue;
        }
        if (line[0] != '{' && line[0] != '}' && lastObject == "NULL")
        {
            lastObject = "";
            int i=0;
            while (i<read_size && line[i] != '\n' && line[i] != '\0' && line[i] != '\r')
            {
                lastObject += line[i];
                i++;
            }
            if(lastObject != "")
            {
                //qDebug() << lastObject << lineCount << " obj";
                AddObject(lastObject);
            }
            continue;
        }

        if (lastObject != "NULL")
        {
            //qDebug() << lastObject << lineCount;
            std::string property;
            std::string value;

            s >> property;
            std::string val = "";
            s >> val;

            while (val != "")
            {
                value += val;
                value += " ";
                val = "";
                s >> val;
            }
            QString str = property.c_str();
            QString str2 =  value.c_str();
            //qDebug() << lastObject << str << str2;
            AddProperty(lastObject, str, str2);
            continue;
        }


    }
    f.close();
    return true;
}
