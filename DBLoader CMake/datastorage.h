
// class yoinked from Engine

#ifndef DATASTORAGE_H
#define DATASTORAGE_H

#include <qcoreapplication.h>
#include <string>
#include <vector>
#include <map>
class DataStorage
{
public:

    std::map<QString,std::map<QString, QString>> data;

    void AddObject(QString ObjectName);
    void PopObject(QString ObjectName);

    void PopProperty(QString ObjectName, QString Property);
    void AddProperty(QString ObjectName, QString Property, QString value = "");

    std::map<QString, QString> GetObject(QString ObjectName);
    std::vector<QString> GetProperties(QString ObjectName);

    QString GetProperty(QString ObjectName, QString Property);
    int GetPropertyAsInt(QString ObjectName, QString Property);
    float GetPropertyAsFloat(QString ObjectName, QString Property);
    bool GetPropertyAsBool(QString ObjectName, QString Property);

    void SetProperty(QString ObjectName, QString Property, QString value = "");
    void SetProperty(QString ObjectName, QString Property, bool value);
    void SetProperty(QString ObjectName, QString Property, int value);
    void SetProperty(QString ObjectName, QString Property, float value);

    void Save(QString filename);
    bool Load(QString filename);
    QString ToString();
};

#endif // DATASTORAGE_H
