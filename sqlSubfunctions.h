#ifndef SQLSUBFUNCTIONS_H
#define SQLSUBFUNCTIONS_H

#include <qobject.h>
inline bool isSpecialSymbol(QChar c)
{
    return c=='!' || c=='=' || c=='*' || c=='&' || c=='%' || c=='(' || c==')' || c=='-' || c=='+' || c=='/' || c==',' || c=='.' || c=='<' || c=='>' || c==';' || c==':' || c=='\"' || c=='\'';
}
inline bool isWord(QString s)
{
    return !s.contains('!') &&
           !s.contains('=') &&
           !s.contains('*') &&
           !s.contains('&') &&
           !s.contains('%') &&
           !s.contains('(') &&
           !s.contains(')') &&
           !s.contains('-') &&
           !s.contains('+') &&
           !s.contains('/') &&
           !s.contains(',') &&
           !s.contains('.') &&
           !s.contains('<') &&
           !s.contains('>') &&
           !s.contains(';') &&
           !s.contains(':');
}
inline bool isNumber(QString s)
{
    for(auto x : s)
    {
        if(!x.isDigit() && x!='.')
            return false;
    }
    return true;
}
// used mostly to not highlight window functions after comma as errors
inline bool isWindowFunc(QString s)
{
    QString str = s.toLower().trimmed();
    return str == "coalesce" ||
           str == "case" ||
           str == "trunc" ||
           str == "date" ||
           str == "lag" ||
           str == "lead" ||
           str == "first" ||
           str == "first_value" ||
           str == "last_value" ||
           str == "(" ||
           str == "sum" ||
           str == "count" ||
           str == "max" ||
           str == "min" ||
           str == "avg"||
           str == "round"||
           str == "last_day"||
           str == "to_char"||
           str == "substr"||
           str == "lower"||
           str == "upper"||
           str == "nvl"||
           str == "account"||
           str == "sysdate"||
           str == "priority"||
           str == "concat"


        ;
}

inline QStringList processBlockToTokens(QString& text)
{

    QStringList tokens;
    int wordstart = 0;
    int wordend = 0;
    bool inword = false;
    QString word = "";
    for(int i =0;i<text.size();i++)
    {
        // word handling
        if(text[i] != ' ' && text[i] != '\n' && text[i] != '\t' && !isSpecialSymbol(text[i]))
        {
            if(!inword)
                wordstart = i;
            word += text[i];
            inword = true;
        }
        else if (inword)
        {
            inword = false;
            wordend = i;
            tokens.push_back(word);
            word.clear();
        }

        //Special symbol handling
        if(isSpecialSymbol(text[i]) && i+1 < text.size() && isSpecialSymbol(text[i+1]) ) //this and next
        {
            QString tword = text[i];
            tword += text[i+1];
            tokens.push_back(tword);
            inword = false;
            word.clear();
            i++;// hop to the next unprocessed symbol
        }
        else if(isSpecialSymbol(text[i])) // only this
        {
            QString tword = text[i];
            tokens.push_back( tword );
            word.clear();
            inword = false;
        }
    }
    if(inword)
    {
        inword = false;
        wordend = text.size();
        tokens.push_back(word);
        word.clear();
    }
    return tokens;
}

#endif // SQLSUBFUNCTIONS_H
