#include "tokenprocessor.h"

#include "sqlSubfunctions.h"

TokenProcessor::TokenProcessor(QObject *parent)
    : QObject{parent}
{}



void TokenProcessor::processText(QString &text)
{
    tokens.clear();
    tokens = processBlockToTokens(text);
}


void TokenProcessor::addFrequencies()
{
    QString prevprevprevtoken = "";
    QString prevprevtoken = "";
    QString prevtoken = "";
    for(auto s : tokens)
    {
        if(s.trimmed().size() < 1 || s.trimmed() ==" ")
            continue;
        if(prevtoken.trimmed().size() == 0)
        {
            prevprevprevtoken = prevprevtoken.replace('\t',' ').replace('\n',' ').trimmed();
            prevprevtoken = prevtoken.replace('\t',' ').replace('\n',' ').trimmed();
            prevtoken = s.replace('\t',' ').replace('\n',' ').trimmed();
            continue;
        }
        else
        {
            QString keystring = prevprevprevtoken.trimmed().toLower() + " " + prevprevtoken.trimmed().toLower() + " " + prevtoken.trimmed().toLower();
            keystring = keystring.trimmed();
            if(keystring.trimmed().size() <=0 || keystring.contains('{')|| keystring.contains('}'))
                continue;
            if(s.trimmed().size() <=0 || s.contains('{')|| s.contains('}'))
                continue;
            int freq = 0;
            if(keystring.size() >=3)
            {
                freq = ds.GetPropertyAsInt(keystring.toLower().toStdString(),s.replace('\t',' ').replace('\n',' ').trimmed().toLower().toStdString()); // get freq
                ds.SetProperty(keystring.toLower().toStdString(),s.replace('\t',' ').replace('\n',' ').trimmed().toLower().toStdString(),freq + 10); //set to freq + 1
            }
            keystring = prevprevtoken.trimmed().toLower() + " " + prevtoken.trimmed().toLower();
            keystring = keystring.trimmed();
            if(keystring.size() >=3)
            {
                freq = ds.GetPropertyAsInt(keystring.toLower().toStdString(),s.trimmed().toLower().toStdString()); // get freq
                ds.SetProperty(keystring.toLower().toStdString(),s.trimmed().toLower().toStdString(),freq + 5); //set to freq + 1
            }

            keystring = prevtoken.trimmed().toLower();
            if(keystring.size() >=3)
            {
                freq = ds.GetPropertyAsInt(keystring.toLower().toStdString(),s.trimmed().toLower().toStdString()); // get freq
                ds.SetProperty(keystring.toLower().toStdString(),s.trimmed().toLower().toStdString(),freq + 2); //set to freq + 1
            }


            prevprevprevtoken = prevprevtoken.trimmed();
            prevprevtoken = prevtoken.trimmed();
            prevtoken = s.trimmed();

            prevprevprevtoken.replace('\n',' ');
            prevprevprevtoken.replace('\t',' ');
            prevprevtoken.replace('\n',' ');
            prevprevtoken.replace('\t',' ');
            prevtoken.replace('\n',' ');
            prevtoken.replace('\t',' ');
            prevprevprevtoken = prevprevprevtoken.trimmed();
            prevprevtoken = prevprevtoken.trimmed();
            prevtoken = prevtoken.trimmed();
        }
    }

}



