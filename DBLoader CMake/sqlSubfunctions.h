#ifndef SQLSUBFUNCTIONS_H
#define SQLSUBFUNCTIONS_H

/*
Functions for easier format translation & stuff
*/

#include <qdatetime.h>
#include <qobject.h>
#include <qvariant.h>

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
           !s.contains(':') &&
           !s.trimmed().contains(' ');
}
inline bool HasLetters(QString s)
{
    return (s.count('!') +
           s.count('=') +
           s.count('*') +
           s.count('&') +
           s.count('%') +
           s.count('(') +
           s.count(')') +
           s.count('-') +
           s.count('+') +
           s.count('/') +
           s.count(',') +
           s.count('.') +
           s.count('<') +
           s.count('>') +
           s.count(';') +
            s.count(':') +
            s.count("\\u") * 2 +
            s.count(' ')) < s.size();
}
inline bool isNumber(QString s)
{
    if(s.count('.') + s.count(',')>1)
        return false;
    for(auto x : s)
    {
        if(!x.isDigit() && x!='.'&& x!=','&& x!='e'&& x!='E')
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
            if(text[i] == '\n')
                tokens.push_back("\n");
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

inline QVariant fixQVariantTypeFormat(QVariant var)
{

    if(var.typeId() == 10) // additional type checking on strings
    {
        QString str = var.toString();
        QDateTime dt = var.toDateTime();
        if(dt.isValid() && !dt.isNull())
        {// datetime
            return QVariant(dt);
        }
        var = str;
        bool hascomas = false;
        bool isdouble = false;
        bool force_double = false;
        bool isint = false;
        bool forcetext = false;
        double doub = var.toDouble(&isdouble);
        var = str;
        int integ = var.toInt(&isint);
        var = str;
        // passes through values like 12312.123123213123123123
        // fix implemented, currently testing
        if(str.size()>15 || (!str.startsWith("0.") || !str.startsWith("0,")) && str.startsWith("0") || str.startsWith("+"))
        {
            doub = false;
            isint = false;
        }
        else if(str.count('.') + str.count(',') == 1 && !((!str.startsWith("0.") || !str.startsWith("0,")) && str.startsWith("0")))
        {// one coma/dot

            QStringList strl = str.replace('.',',').split(',');

            if(str.contains(','))
                hascomas = true;

            if(isNumber(str))
            {
                force_double = true;
            }
            strl = str.replace(',','.').split('.');

            if(strl.size()==2)
            {
                bool ok1 = false;
                bool ok2 = false;
                QVariant(strl[0]).toInt(&ok1);
                QVariant(strl[1]).toInt(&ok2);
                if(ok1 && ok2)
                    force_double = true;
            }
        }
        if(force_double )
        {
            if(!hascomas)
                return QVariant(var.toDouble());
            else
                return QVariant((var.toString().replace(',','.')).toDouble());
        }
        else
        {
            if((!dt.isValid() || dt.isNull()) && isdouble && str.size()>9)
            {
                isdouble = false;
                forcetext = true;
            }
            if(!forcetext)
            {

                if(dt.isValid() && !dt.isNull())
                {// datetime
                    return QVariant(dt);
                }
                else if(isdouble && str.count('.') + str.count(',') > 0)
                {
                    return QVariant(var.toDouble());
                }
                else if(isint)
                {
                    return QVariant(var.toLongLong());
                }
            }
            else
                return str;
        }
    }
    return var;
}

thread_local inline int fixQStringType_lasttype = 10;
inline QString fixQStringType(QString str)
{

    fixQStringType_lasttype = 10;

    if((str.size()>=2 && !(str[0]==QChar('0') && str[1]!=QChar('.')&& str[1]!=QChar(',')) && str[0]!=QChar('+')) || str.size()< 2)
    {
        int cnt = str.count('.');
        bool isdouble = false;
        if(cnt == 1)
        {
            str.toDouble(&isdouble);
            if(isdouble)
            {
                fixQStringType_lasttype = 6;
                return str;
            }
        }
        else if (cnt > 1)
        {

            if(str.size() == 19 || str.size() == 23 || str.size() == 10)//2000-01-01 10:10:10 || 2000-01-01 10:10:10.000 || 2000-01-01
            {
                if((str[4] == '-' || str[4] == '.') && (str[7] == '-' || str[7] == '.'))
                {
                    fixQStringType_lasttype = 16; // probably is date
                    if(str.size()<19)//its only date;
                    {
                        str.resize(19);
                        str[10] = ' ';
                        str[11] = '0';
                        str[12] = '0';
                        str[13] = ':';
                        str[14] = '0';
                        str[15] = '0';
                        str[16] = ':';
                        str[17] = '0';
                        str[18] = '0';
                    }
                    else
                        str.resize(19);

                    return str.replace('T',' ');
                }
            }
            return str;
        }
        cnt+= str.count(',');

        if(cnt == 1)
        {
            QString doub = str.replace(',','.');
            doub.toDouble(&isdouble);
            if(isdouble)
            {
                fixQStringType_lasttype = 6;
                return str;
            }
        }
        else if (cnt == 0)
        {
            bool ok = false;
            str.toInt(&ok);

            if(ok)
            {
                fixQStringType_lasttype = 6;
                return str;
            }
        }

        if(str.size() == 19 || str.size() == 23 || str.size() == 10)//2000-01-01 10:10:10 || 2000-01-01 10:10:10.000 || 2000-01-01
        {
            if((str[4] == '-' || str[4] == '.') && (str[7] == '-' || str[7] == '.'))
            {
                fixQStringType_lasttype = 16; // probably is date
                if(str.size()<19)//its only date;
                {
                    str.resize(19);
                    str[10] = ' ';
                    str[11] = '0';
                    str[12] = '0';
                    str[13] = ':';
                    str[14] = '0';
                    str[15] = '0';
                    str[16] = ':';
                    str[17] = '0';
                    str[18] = '0';
                }
                else
                    str.resize(19);

                return str.replace('T',' ');
            }
        }
        return str;
    }
    else
    {// string or date

        if(str.size() == 19 || str.size() == 23 || str.size() == 10)//2000-01-01 10:10:10 || 2000-01-01 10:10:10.000 || 2000-01-01
        {
            if((str[4] == '-' || str[4] == '.') && (str[7] == '-' || str[7] == '.'))
            {
                fixQStringType_lasttype = 16; // probably is date
                if(str.size()<19)//its only date;
                {
                    str.resize(19);
                    str[10] = ' ';
                    str[11] = '0';
                    str[12] = '0';
                    str[13] = ':';
                    str[14] = '0';
                    str[15] = '0';
                    str[16] = ':';
                    str[17] = '0';
                    str[18] = '0';
                }
                else
                    str.resize(19);

                return str.replace('T',' ');
            }
        }
        return str;
    }

    return str;
}




#endif // SQLSUBFUNCTIONS_H
