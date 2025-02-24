#include "highlighter.h"
#include "Patterns.h"
#include <qdir.h>
#include <qsqlerror.h>
#include <qsqlquery.h>
#include <qsqlrecord.h>
#include <QSqlDriver>
Highlighter::Highlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    HighlightingRule rule;


    NameFormat.setForeground( QColor(Qt::cyan));
    NameFormat.setFontWeight(QFont::Bold);


    keywordFormat.setForeground( QColor::fromRgbF(0.25f,0.55f,0.95f,1.0f));
    keywordFormat.setFontWeight(QFont::Bold);

    NameFormat.setForeground( QColor(Qt::GlobalColor::magenta).lighter(150));
    NameFormat.setFontWeight(QFont::Bold);


    NameFormat.setForeground( QColor::fromRgbF(0.85f,0.45f,0.15f,1.0f));
    NameFormat.setFontWeight(QFont::Bold);
    classFormat.setFontWeight(QFont::Bold);
    classFormat.setForeground( QColor::fromRgbF(0.65f,0.25f,0.65f,1.0f));

    quotationFormat.setForeground(QColor::fromRgbF(0.6f,0.5f,0.25f,1.0f));

    functionFormat.setFontItalic(true);
    functionFormat.setForeground(Qt::blue);

    singleLineCommentFormat.setForeground(QColor(Qt::green).lighter(90));

    multiLineCommentFormat.setForeground(QColor::fromRgbF(0.15f,0.75f,0.15f,1.0f));

}


void Highlighter::highlightBlock(const QString &text)
{
    textintervals.clear();


    QChar last_char = ' ';
    TextInterval te;
    bool added = false;
    for(int i=0;i<=text.size();i++)
    {
        if(added)
        {
            textintervals.back().is_number = true;
            for(auto chr : textintervals.back().text)
            {
                if(!chr.isNumber())
                    if(chr != '-' || chr != '.'|| chr != ','|| chr != '('|| chr != ')'|| chr != ';')
                    {
                        textintervals.back().is_number = false;

                        break;
                    }

            }

        }
        added = false;
        if(te.type!=0)
            te.end = i;
        if(te.type ==-1)
            if( (text[i] == ' ' ||text[i] == '=' || text[i] == '.'|| text[i] == ','|| text[i] == '('|| text[i] == ')'|| text[i] == '\''|| text[i] == '\n'|| text[i] == ';'  || i == text.size()))
            {
                te.end = i;
                for(int a = te.start; a < te.end;a++)
                    te.text.push_back(text[a]);
                textintervals.push_back(te);
                te.end = 0;
                te.start = 0;
                te.type = 0;
                te.PrevWord = te.text;
                te.text = "";
                added = true;
                last_char = text[i];
                continue;
            }
        if(te.type == 1)
            if((text[i] == '\n' || i == text.size()))
            {
                te.end = i;
                for(int a = te.start; a <= te.end;a++)
                    te.text.push_back(text[a]);
                textintervals.push_back(te);
                te.end = 0;
                te.start = 0;
                te.type = 0;
                te.PrevWord = te.text;
                te.text = "";
                added = true;
                last_char = text[i];
                continue;
            }
        if(te.type == 2)
            if(text[i] == '\'')
            {
                te.end = i;
                for(int a = te.start; a <= te.end;a++)
                    te.text.push_back(text[a]);
                textintervals.push_back(te);
                te.end = 0;
                te.start = 0;
                te.type = 0;
                te.PrevWord = te.text;
                te.text = "";
                added = true;
                last_char = text[i];
                continue;
            }

        if(te.type == 0)
        {
            if(text[i] == '\'')
            {
                te.end = i+1;
                te.start = i;
                te.type = 2; // text
                last_char = text[i];
                continue;
            }
            else if((last_char == '-') && (text[i]== '-'))
            {
                te.end = i;
                te.start = i-1;
                te.type = 1; // comment
                last_char = text[i];
                continue;
            }
            else if((last_char == ' ' || last_char == '.'||last_char == '<'||last_char == '>'|| last_char == ',' || last_char == '=' || last_char == '('|| last_char == ')' || last_char == ';'   || i == 0) && ((text[i]!= ' ' && text[i]!= '-' && text[i]!= '(') || i == text.size()))
            {
                te.end = i;
                te.start = i;
                te.type = -1;
                last_char = text[i];
                continue;
            }
        }
        last_char = text[i];
    }


    bool prevWordIsTable = false;



    TableAliasMapPerRow[currentBlock().blockNumber()].clear();
    if(PostgresStyle) // convert strings back to normal, without ""
    {
        for(int  i =0;i < textintervals.size();i++)
        {
            QStringList sl = textintervals[i].text.split('"');
            if(sl.size() > 1)
                textintervals[i].text = sl[1];
            else if(sl.size() >0)
                textintervals[i].text = sl[0];
            sl = textintervals[i].PrevWord.split('"');
            if(sl.size() > 1)
                textintervals[i].PrevWord = sl[1];
            else if(sl.size() >0)
                textintervals[i].PrevWord = sl[0];
        }

    }
    for(auto i : textintervals)
    {
        if((prevWordIsTable) && !keywordPatterns.contains(i.text)&& !ColumnMap.contains(i.text)
            && i.text.size()>0&& i.text[0].isLetter())
            TableAliasMapPerRow[currentBlock().blockNumber()][i.text] = i;

        if(TableColumnMap.contains(i.text))
            prevWordIsTable = true;
        else
            prevWordIsTable = false;

    }

    TableColumnAliasMap.clear();
    TableColumnAliasMap_lower.clear();
    for(auto ta : TableAliasMapPerRow)
    {
        for(auto i : ta.keys())
        {
            TableColumnAliasMap[i] = ta[i].PrevWord;
            TableColumnAliasMap_lower[i.toLower()] = ta[i].PrevWord.toLower();
        }
    }



    if(PostgresStyle)
        for(auto i : textintervals)
    {
        QTextCharFormat format;
        format.setForeground( QColor::fromRgbF(0.95f,0.95f,0.85f,1.0f));

        if(i.type == -1)
        {
            if(keywordPatterns.contains(i.text.toLower()))
            {
                format = keywordFormat;
            }
            else if(TableColumnMap.contains(i.text) || TableColumnAliasMap.contains(i.text))
            {
                format = classFormat;
            }
            else if(TableColumnMap.contains(i.PrevWord))
            {if(TableColumnMap[i.PrevWord].contains(i.text))
                {
                    format = NameFormat;
                }}
            else if(TableColumnAliasMap.contains(i.PrevWord))
            {if(TableColumnMap[TableColumnAliasMap[i.PrevWord]].contains(i.text))
                {
                    format = NameFormat;
                }}
            else if(ColumnMap.contains(i.text))
            {
                format = NameFormat;
            }

            if(i.text == "*" || i.text == "=" || i.text == ">" || i.text == "<" || i.text == "<>" || i.text == "!=" || i.text == "+" || i.text == "-" )
                format = keywordFormat;
            if(i.text == ";")
                format = keywordFormat;
            if(i.text == "(" ||i.text == ")")
                format = keywordFormat;

            if(i.is_number)
            {
                format = keywordFormat;
                format.setForeground( QColor::fromRgbF(0.45f,0.65f,0.85f,1.0f));
            }

        }
        else if(i.type == 1)
            format = multiLineCommentFormat;
        else if(i.type == 2 || i.type == 3)
            format = quotationFormat;
        setFormat(i.start,i.end, format);
    }
    else
    {
        for(auto i : textintervals)
        {
            QTextCharFormat format;
            format.setForeground( QColor::fromRgbF(0.95f,0.95f,0.85f,1.0f));

            if(i.type == -1)
            {
                if(keywordPatterns.contains(i.text.toLower(),Qt::CaseInsensitive))
                {
                    format = keywordFormat;
                }
                else if(TableColumnMap_lower.contains(i.text.toLower()) || TableColumnAliasMap_lower.contains(i.text.toLower()))
                {
                    format = classFormat;
                }
                else if(TableColumnMap_lower.contains(i.PrevWord.toLower()))
                {if(TableColumnMap_lower[i.PrevWord.toLower()].contains(i.text.toLower()))
                    {
                        format = NameFormat;
                    }}
                else if(TableColumnAliasMap_lower.contains(i.PrevWord.toLower()))
                {if(TableColumnMap_lower[TableColumnAliasMap[i.PrevWord.toLower()]].contains(i.text.toLower()))
                    {
                        format = NameFormat;
                    }}
                else if(ColumnMap_lower.contains(i.text.toLower()))
                {
                    format = NameFormat;
                }

                if(i.text == "*" || i.text == "=" || i.text == ">" || i.text == "<" || i.text == "<>" || i.text == "!=" || i.text == "+" || i.text == "-" )
                    format = keywordFormat;
                if(i.text == ";")
                    format = keywordFormat;
                if(i.text == "(" ||i.text == ")")
                    format = keywordFormat;

                if(i.is_number)
                {
                    format = keywordFormat;
                    format.setForeground( QColor::fromRgbF(0.45f,0.65f,0.85f,1.0f));
                }

            }
            else if(i.type == 1)
                format = multiLineCommentFormat;
            else if(i.type == 2 || i.type == 3)
                format = quotationFormat;
            setFormat(i.start,i.end, format);
        }
    }


}


void Highlighter::UpdateTableColumns(QSqlDatabase* db, QString dbname)
{
    ColumnMap.clear();
    TableAliasMapPerRow.clear();
    TableColumnAliasMap.clear();
    TableColumnMap.clear();

    TableColumnAliasMap_lower.clear();
    TableColumnMap_lower.clear();

    dbPatterns.clear();
    TableColumnDS.data.clear();
    if(!QSLiteStyle)
    {
    QStringList strl = db->databaseName().split(';');
    QString filename = dbname;

    filename += QString(".txt");
    if(QFile::exists(filename))
    {
        TableColumnDS.Load(filename.toStdString());
    }
    else
    {
        QString sql = "";
        QFile fl("user_get_tables_columns_script.sql");
        fl.open(QFile::OpenModeFlag::ReadOnly);
        sql = fl.readAll();

        db->open();
        QSqlQuery q(*db);


        if(q.exec(sql))
        {

            while(q.next())
            {
                QString c_a = "";
                QString c_b = "";
                for(int a=0, total = q.record().count();a<total;++a)
                {
                    if(a==0)
                        c_a = q.value(a).toString();
                    if(a==1)
                        c_b = q.value(a).toString();
                }
                TableColumnDS.data[c_a.toStdString()][c_b.toStdString()];
            }
            qDebug()<<TableColumnDS.data.size()  << filename;
        }
        else
            qDebug()<<"error in sql: \n " << sql;
        TableColumnDS.Save(filename.toStdString());
        qDebug()<<filename;
    }
    }
    else
    {

        db->open();
        QStringList tblnames = db->driver()->tables(QSql::AllTables);
        for(auto tbl : tblnames)
        {
            TableColumnDS.data[tbl.toStdString()];
        }
        for(auto tbn : tblnames)
        {
            QSqlRecord rec = db->driver()->record(tbn);
            int reccount = rec.count();
            for(int a=0,total = reccount; a<total;a++)
            {
                TableColumnDS.data[tbn.toStdString()][rec.fieldName(a).toStdString()];
            }
        }

    }

    for(auto x : TableColumnDS.data)
    {
        for(auto y : x.second)
        {
            TableColumnMap[x.first.c_str()][y.first.c_str()] = true;
            TableColumnMap_lower[QVariant(x.first.c_str()).toString().toLower()][QVariant(y.first.c_str()).toString().toLower()] = true;
            ColumnMap[y.first.c_str()] = true;
            ColumnMap_lower[QVariant(y.first.c_str()).toString().toLower()] = true;

        }
        dbPatterns.push_back(x.first.c_str());
    }
}
