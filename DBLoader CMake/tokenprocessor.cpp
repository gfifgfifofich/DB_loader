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
    QStringList prevtokens;
    int depth = 30;

    bool in_qutes = false;
    bool in_doublequtes = false;
    bool inComment = false;
    for(auto s : tokens)
    {
        // include comments or not
        if(s.contains("--"))
            inComment=true;
        if(inComment && s.contains("\n"))
        {
            inComment = false;
            continue;
        }
        if(inComment)
            continue;

        // include data from quotes, or not
        int quotes = s.count('\'');
        if(quotes % 2 == 1)
        {
            in_qutes = !in_qutes;
            continue;
        }
        if(in_qutes)
            continue;

        // include data from doublequotes, or not
        int double_quotes = s.count('"');
        if(quotes % 2 == 1)
        {
            in_doublequtes = !in_doublequtes;
            continue;
        }
        if(in_doublequtes)
            continue;



        if(prevtokens.size() > 0)
        {
            QString str = s.replace('\n',' ').replace('\t',' ').replace('\u0000',' ').toLower().trimmed();
            if((!isNumber(str) || str == "."|| (!isWord(str) && !isNumber(str))) && !str.contains('{')&& !str.contains(';') && !str.contains('}'))


            //isWord(str) && !isNumber(str) || (str.size()>2 && !isNumber(str)) || str == '.'||  str == '('|| str == ')'|| str == '='|| str == "--" || str == ">=" || str == "<=")
                //|| (!isNumber(str) || str == "."|| (!isWord(str) && !isNumber(str))) && !str.contains('{')&& !str.contains(';') && !str.contains('}')
            {
                QString tokenkey = "";

                int wordcount = 0;
                for (int a =  prevtokens.size()-1; a >= 0 ;a--)
                {
                    wordcount ++;
                    tokenkey = prevtokens[a] + " " + tokenkey;
                    tokenkey = tokenkey.trimmed();
                    if(wordcount >= 0)
                    {
                        float val =pow(1.5,a) * 0.1f * sqrt(str.size());
                        float freq = freqs[tokenkey][str];//ds.GetPropertyAsFloat(tokenkey.toStdString(),str.toStdString()); // get freq

                        freq += val;


                        if(freq > 10000)
                            freq = 10000;
                        freqs[tokenkey][str] = freq;//ds.SetProperty(tokenkey.toStdString(),str.toStdString(),freq); //set to freq + val
                    }
                }
                prevtokens.push_back(str);
                if(prevtokens.size()>depth)
                    prevtokens.pop_front();
            }
        }
        else
            prevtokens.push_back(s.replace('\n',' ').replace('\t',' ').replace('\u0000',' ').toLower().trimmed());

    }


}



