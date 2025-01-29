#include "datastorage.h"
#include <fstream>
#include <iostream>

void DataStorage::AddObject(std::string ObjectName)
{
    std::map<std::string, std::string> a;
    data.insert(std::pair<std::string, std::map<std::string, std::string>>(ObjectName, a));
}


void DataStorage::AddProperty(std::string ObjectName, std::string Property, std::string value)
{
    data[ObjectName].insert(std::pair < std::string, std::string>(Property, value));
}


void DataStorage::PopProperty(std::string ObjectName, std::string Property)
{
    data[ObjectName].erase(Property);

}
void DataStorage::PopObject(std::string ObjectName)
{
    data.erase(ObjectName);
}


std::map<std::string, std::string> DataStorage::GetObject(std::string ObjectName)
{
    return data[ObjectName];
}

std::vector<std::string> DataStorage::GetProperties(std::string ObjectName)
{
    std::vector<std::string> Properties;
    for (auto i : data[ObjectName])
        Properties.push_back(i.first);

    return Properties;
}

std::string DataStorage::GetProperty(std::string ObjectName, std::string Property)
{
    std::string prop = data[ObjectName][Property];
    while(prop.size()>0 && prop[prop.size()-1] == ' ')
        prop.pop_back();
    return prop;
}

int DataStorage::GetPropertyAsInt(std::string ObjectName, std::string Property)
{
    std::strstream ss;
    ss<< data[ObjectName][Property];
    int i = 0;
    ss >> i;
    return i;
}
float DataStorage::GetPropertyAsFloat(std::string ObjectName, std::string Property)
{
    std::strstream ss;
    ss << data[ObjectName][Property];
    float f = 0.0f;
    ss >> f;
    return f;
}
bool DataStorage::GetPropertyAsBool(std::string ObjectName, std::string Property)
{
    std::strstream ss;
    ss << data[ObjectName][Property];
    bool b = 0;
    ss >> b;
    return b;
}

void DataStorage::SetProperty(std::string ObjectName, std::string Property, std::string value)
{
    data[ObjectName][Property] = value;
}

void DataStorage::SetProperty(std::string ObjectName,std::string Property, bool value)
{
    data[ObjectName][Property] = std::to_string(value);

}
void DataStorage::SetProperty(std::string ObjectName,std::string Property, int value)
{
    data[ObjectName][Property] = std::to_string(value);

}
void DataStorage::SetProperty(std::string ObjectName,std::string Property, float value)
{
    data[ObjectName][Property] = std::to_string(value);
}


void DataStorage::Save(std::string filename)
{
    std::ofstream File(filename);

    for (auto i : data)
    {
        File << i.first;
        File << "\n{";

        for (auto p : i.second)
        {
            File << "\n	" << p.first << " " << p.second << "";
        }

        File << "\n}\n";
    }
    File.close();
}

std::string DataStorage::ToString()
{
    std::string File = "";

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

void DataStorage::Load(std::string filename)
{
    data.clear();

    std::ifstream f(filename);
    if (!f.is_open())
    {
        std::cout << "ERROR LOADING SaveFile: Unable to load " << filename;
        return;
    }

    std::string lastObject = "NULL";
    while (!f.eof())
    {
        char junk;
        char line[256];
        f.getline(line, 256);
        std::strstream s;
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
            s >> lastObject;
            if(lastObject != "")
                AddObject(lastObject);
            continue;
        }

        if (lastObject != "NULL")
        {
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
            AddProperty(lastObject, property, value);
            continue;
        }


    }


}
