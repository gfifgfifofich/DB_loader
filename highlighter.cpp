#include "highlighter.h"
#include "Patterns.h"
#include <qdir.h>
#include <qsqlerror.h>
#include <qsqlquery.h>
#include <qsqlrecord.h>
#include <QSqlDriver>
#include "datastorage.h"

inline DataStorage userDS;

/*
    ColumnBold true
    ColumnColor 217,115,33,255
    ColumnItalic false
    ColumnSize 10
    CommentBold false
    CommentColor 33,191,33,255
    CommentItalic false
    CommentSize 10
    KeyWordBold true
    KeyWordColor 64,140,242,255
    KeyWordItalic false
    KeyWordSize 10
    NameBold true
    NameColor 166,64,166,255
    NameItalic false
    NameSize 10
    quotationBold false
    quotationColor 150,128,64,255
    quotationItalic false
    quotationSize 10
    BracketHighlightColor 35,3,3,255
*/
Highlighter::Highlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    HighlightingRule rule;

    if(userDS.Load("userdata.txt"))
    {
        if(QString(userDS.data["UserTheme"]["KeyWordItalic"].c_str()).trimmed().toLower() == "true")
            keywordFormat.setFontItalic(true);
        if(QString(userDS.data["UserTheme"]["KeyWordBold"].c_str()).trimmed().toLower() == "true")
            keywordFormat.setFontWeight(QFont::Bold);
        QStringList strl = QString(userDS.data["UserTheme"]["KeyWordColor"].c_str()).split(',');
        QColor col = QColor::fromRgbF(0.25f,0.55f,0.95f,1.0f);
        for(int i=0;i<strl.size();i++)
        {
            if(i==0) col.setRed(QVariant(strl[i]).toInt());
            if(i==1) col.setGreen(QVariant(strl[i]).toInt());
            if(i==2) col.setBlue(QVariant(strl[i]).toInt());
            if(i==3) col.setAlpha(QVariant(strl[i]).toInt());
        }
        keywordFormat.setForeground( col);
        keywordFormat.setFontPointSize(QVariant(userDS.data["UserTheme"]["KeyWordSize"].c_str()).toInt());



        if(QString(userDS.data["UserTheme"]["ColumnItalic"].c_str()).trimmed().toLower() == "true") // columns
        NameFormat.setFontItalic(true);
        if(QString(userDS.data["UserTheme"]["ColumnBold"].c_str()).trimmed().toLower() == "true")
            NameFormat.setFontWeight(QFont::Bold);
        strl = QString(userDS.data["UserTheme"]["ColumnColor"].c_str()).split(',');
        col = QColor::fromRgbF(0.85f,0.45f,0.15f,1.0f);
        for(int i=0;i<strl.size();i++)
        {
            if(i==0) col.setRed(QVariant(strl[i]).toInt());
            if(i==1) col.setGreen(QVariant(strl[i]).toInt());
            if(i==2) col.setBlue(QVariant(strl[i]).toInt());
            if(i==3) col.setAlpha(QVariant(strl[i]).toInt());
        }
        NameFormat.setForeground( col );
        NameFormat.setFontPointSize(QVariant(userDS.data["UserTheme"]["ColumnSize"].c_str()).toInt());



        if(QString(userDS.data["UserTheme"]["NameItalic"].c_str()).trimmed().toLower() == "true") // tables
        classFormat.setFontItalic(true);
        if(QString(userDS.data["UserTheme"]["NameBold"].c_str()).trimmed().toLower() == "true")
            classFormat.setFontWeight(QFont::Bold);
        strl = QString(userDS.data["UserTheme"]["NameColor"].c_str()).split(',');
        col = QColor::fromRgbF(0.65f,0.25f,0.65f,1.0f);
        for(int i=0;i<strl.size();i++)
        {
            if(i==0) col.setRed(QVariant(strl[i]).toInt());
            if(i==1) col.setGreen(QVariant(strl[i]).toInt());
            if(i==2) col.setBlue(QVariant(strl[i]).toInt());
            if(i==3) col.setAlpha(QVariant(strl[i]).toInt());
        }
        classFormat.setForeground( col );
        classFormat.setFontPointSize(QVariant(userDS.data["UserTheme"]["NameSize"].c_str()).toInt());



        if(QString(userDS.data["UserTheme"]["quotationItalic"].c_str()).trimmed().toLower() == "true")
        quotationFormat.setFontItalic(true);
        if(QString(userDS.data["UserTheme"]["quotationBold"].c_str()).trimmed().toLower() == "true")
            quotationFormat.setFontWeight(QFont::Bold);
        strl = QString(userDS.data["UserTheme"]["quotationColor"].c_str()).split(',');
        col = QColor::fromRgbF(0.6f,0.5f,0.25f,1.0f);
        for(int i=0;i<strl.size();i++)
        {
            if(i==0) col.setRed(QVariant(strl[i]).toInt());
            if(i==1) col.setGreen(QVariant(strl[i]).toInt());
            if(i==2) col.setBlue(QVariant(strl[i]).toInt());
            if(i==3) col.setAlpha(QVariant(strl[i]).toInt());
        }
        quotationFormat.setForeground(col);
        quotationFormat.setFontPointSize(QVariant(userDS.data["UserTheme"]["quotationSize"].c_str()).toInt());



        if(QString(userDS.data["UserTheme"]["CommentItalic"].c_str()).trimmed().toLower() == "true")
        multiLineCommentFormat.setFontItalic(true);
        if(QString(userDS.data["UserTheme"]["CommentBold"].c_str()).trimmed().toLower() == "true")
            multiLineCommentFormat.setFontWeight(QFont::Bold);
        strl = QString(userDS.data["UserTheme"]["CommentColor"].c_str()).split(',');
        col = QColor::fromRgbF(0.15f,0.75f,0.15f,1.0f);
        for(int i=0;i<strl.size();i++)
        {
            if(i==0) col.setRed(QVariant(strl[i]).toInt());
            if(i==1) col.setGreen(QVariant(strl[i]).toInt());
            if(i==2) col.setBlue(QVariant(strl[i]).toInt());
            if(i==3) col.setAlpha(QVariant(strl[i]).toInt());
        }
        multiLineCommentFormat.setForeground(col);
        multiLineCommentFormat.setFontPointSize(QVariant(userDS.data["UserTheme"]["CommentSize"].c_str()).toInt());




        if(userDS.data["UserTheme"]["ColumnItalic"] == "true")//?
            functionFormat.setFontItalic(true);
        if(false)
            functionFormat.setFontWeight(QFont::Bold);
        functionFormat.setForeground(Qt::blue);

    }
    else
    {
        keywordFormat.setFontItalic(false);
        if(true)
            keywordFormat.setFontWeight(QFont::Bold);
        keywordFormat.setForeground( QColor::fromRgbF(0.25f,0.55f,0.95f,1.0f));
        keywordFormat.setFontPointSize(10);

        NameFormat.setFontItalic(false);
        if(true)
            NameFormat.setFontWeight(QFont::Bold);
        NameFormat.setForeground( QColor::fromRgbF(0.85f,0.45f,0.15f,1.0f));

        classFormat.setFontItalic(false);
        if(true)
            classFormat.setFontWeight(QFont::Bold);
        classFormat.setForeground( QColor::fromRgbF(0.65f,0.25f,0.65f,1.0f));

        quotationFormat.setFontItalic(false);
        if(false)
            quotationFormat.setFontWeight(QFont::Bold);
        quotationFormat.setForeground(QColor::fromRgbF(0.6f,0.5f,0.25f,1.0f));

        functionFormat.setFontItalic(true);
        if(false)
            functionFormat.setFontWeight(QFont::Bold);
        functionFormat.setForeground(Qt::blue);

        multiLineCommentFormat.setFontItalic(true);
        if(false)
            multiLineCommentFormat.setFontWeight(QFont::Bold);
        multiLineCommentFormat.setForeground(QColor::fromRgbF(0.15f,0.75f,0.15f,1.0f));
    }


    singleLineCommentFormat.setForeground(QColor(Qt::green).lighter(90));
}

bool isSpecialSymbol(QChar c)
{
    return c=='!' || c=='=' || c=='*' || c=='&' || c=='%' || c=='(' || c==')' || c=='-' || c=='+' || c=='/' || c==',' || c=='.' || c=='<' || c=='>' || c==';' || c==':' || c=='\"' || c=='\'';
}
bool isWord(QString s)
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
bool isNumber(QString s)
{
    for(auto x : s)
    {
        if(!x.isDigit() && x!='.')
            return false;
    }
    return true;
}
// used mostly to not highlight window functions after comma as errors
bool isWindowFunc(QString s)
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
           str == "upper"


        ;
}

void Highlighter::highlightBlock(const QString &text)
{

    int bn = currentBlock().blockNumber();
    currentBlock().setUserState(bn);


    tokens[bn].clear();

    QStringList words = text.split(' ');
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
            tokens[currentBlock().blockNumber()].push_back({wordstart,wordend,word});
            word.clear();
        }

        //Special symbol handling
        if(isSpecialSymbol(text[i]) && i+1 < text.size() && isSpecialSymbol(text[i+1]) ) //this and next
        {
            QString tword = text[i];
            tword += text[i+1];
            tokens[currentBlock().blockNumber()].push_back( {i, i+1, tword } );
            word.clear();
            i++;// hop to the next unprocessed symbol
        }
        else if(isSpecialSymbol(text[i])) // only this
        {
            QString tword = text[i];
            tokens[currentBlock().blockNumber()].push_back( {i, i, tword} );
        }
    }
    if(inword)
    {
        inword = false;
        wordend = text.size();
        tokens[currentBlock().blockNumber()].push_back({wordstart,wordend,word});
        word.clear();
    }

    textintervals.clear();

    TableAliasMapPerRow[bn].clear();
    for (int i = 2 ; i < tokens[bn].size() ; i++)
    {
        int step = 1;
        if (PostgresStyle)
            step = 2; // skip the ""

        QString Prevword = tokens[bn][i-step].text;
        QString word = tokens[bn][i].text;
        if(!PostgresStyle)
        {
            word = word.toLower();
            Prevword  = Prevword.toLower();
        }

        if (TableColumnMap.contains(Prevword ) &&
            !keywordPatterns.contains(word ,Qt::CaseInsensitive) &&
            !ColumnMap.contains(word))
        {
            TextInterval tmpti;
            tmpti.start = tokens[bn][i].start;
            tmpti.end = tokens[bn][i].end;
            tmpti.text = tokens[bn][i].text;
            tmpti.PrevWord = tokens[bn][i-step].text;
            TableAliasMapPerRow[bn][word] = tmpti;
            qDebug()<<"creating alias " <<  tmpti.PrevWord << tmpti.text;
        }

    }

    TableColumnAliasMap.clear();
    TableColumnAliasMap_lower.clear();
    for(auto ta : TableAliasMapPerRow)
    {
        for(auto i : ta.keys())
        {
            if(PostgresStyle)
                TableColumnAliasMap[i] = ta[i].PrevWord;
            else
                TableColumnAliasMap[i.toLower()] = ta[i.toLower()].PrevWord.toLower();

        }
    }



    for(int i=0;i < tokens[bn].size();i++)
    {
        if(tokens[bn][i].start<0 || tokens[bn][i].end < 0)
            continue;
        QTextCharFormat format;
        format.setForeground( QColor::fromRgbF(0.95f,0.95f,0.85f,1.0f));

        int aliasstep = 1;
        if(PostgresStyle)
            aliasstep = 2 ;

        QString word = tokens[bn][i].text;
        QString prevword = "";
        if(i>0)
            prevword = tokens[bn][i-1].text;
        QString SteppedWord = "";
        if(i - aliasstep >= 0)
            SteppedWord = tokens[bn][i-aliasstep].text;
        if(!PostgresStyle)
        {
            word = word.toLower();
            SteppedWord = SteppedWord .toLower();
            prevword = prevword .toLower();
        }
        if(/*is word*/ isWord(word))
        {
            bool addQuotesToToken = false;
            if(ColumnMap_lower.contains(word.toLower()))
            {
                format = NameFormat;
                addQuotesToToken = true;
            }
            if(keywordPatterns.contains(word,Qt::CaseInsensitive) && !prevword.contains('.'))
            {
                format = keywordFormat;
            }
            else if(TableColumnMap.contains(word) || TableColumnAliasMap.contains(word))
            {
                format = classFormat;
                addQuotesToToken = true;
            }
            else if(TableColumnMap.contains(SteppedWord ))
                {if(TableColumnMap[SteppedWord].contains(word))
                {
                    format = NameFormat;
                    addQuotesToToken = true;
                }}
            else if(TableColumnAliasMap.contains(SteppedWord))
                {if(TableColumnMap[TableColumnAliasMap[SteppedWord]].contains(word))
                {
                    format = NameFormat;
                    addQuotesToToken = true;
                }}

            if(addQuotesToToken && i-1 > 0 && i+1 < tokens[bn].size())
            {// add "" symbols to token
                if((tokens[bn][i-1].text == "\"" || tokens[bn][i-1].text == ".\"") &&
                    i+1 < tokens[bn].size() &&
                    (tokens[bn][i+1].text == "\"" || tokens[bn][i+1].text == "\";" ) )
                {
                    tokens[bn][i].start--;
                    tokens[bn][i].end++;
                    tokens[bn][i+1].start = -1;
                    tokens[bn][i+1].end = -1;
                }
            }


            if(isNumber(word))
            {
                format = keywordFormat;
                format.setForeground( QColor::fromRgbF(0.45f,0.65f,0.85f,1.0f));
            }

        }
        else if(word.size() == 1 && isSpecialSymbol(word[0]))
            format = keywordFormat;
        else if(/*comment*/false)
            format = multiLineCommentFormat;
        else if(/*quotation*/ false)
            format = quotationFormat;
        setFormat(tokens[bn][i].start,tokens[bn][i].end - tokens[bn][i].start, format);
    }

    bool quote = false;
    int quote_start = 0;

    int bracketscount = 0;
    std::vector <int> bracketstarts;
    bool comment = false;


    for(int i = 0;i < tokens[bn].size();i++)
    {
        if(tokens[bn][i].text.contains("--") || (i > 0 && tokens[bn][i].text.startsWith('-') && tokens[bn][i-1].text.endsWith('-') && tokens[bn][i-1].end +1 == tokens[bn][i].start))
            comment = true;
        if(comment)
        {
            setFormat(tokens[bn][i].start,text.size(), multiLineCommentFormat);
            break;
        }
        if(tokens[bn][i].text.contains('('))
        {
            bracketscount += tokens[bn][i].text.count('(');
            bracketstarts.push_back(tokens[bn][i].start);
        }
        if(tokens[bn][i].text.contains(')'))
        {
            bracketscount -= tokens[bn][i].text.count(')');
            if(bracketstarts.size() > 0)
            bracketstarts.pop_back();
        }
        if(tokens[bn][i].text.contains('\''))
        {
            if(tokens[bn][i].text.count('\'') % 2 != 0)
            {
                quote = !quote;
                if(quote)
                    quote_start =  tokens[bn][i].end;
                if(!quote)
                    setFormat(quote_start, (tokens[bn][i].start + 1) - quote_start , quotationFormat);

                continue;
            }
        }
    }


    QList<int> tokenKeys = tokens.keys();
    if(bracketscount > 0)
    {// not found closing braket on current block
        bool goodenough = false;
        bool toofar = false;
        //Find on other blocks
        int dist = 0;
        for(auto k : tokenKeys)
        {
            if(k > bn)// blocks after this block
            {
                for(auto s : tokens[k])
                {
                    if(s.text.contains('(') )
                    {
                        bracketscount += s.text.count('(');
                    }
                    if(s.text.contains(')'))
                    {
                        bracketscount -= s.text.count(')');
                    }
                    if(bracketscount <= 0)// this bracket got closed somewhere, so we no longer care about what comes further
                    {
                        goodenough = true;
                        break;
                    }
                    //dist +=1;
                    //if(dist > 500)
                    //{
                    //    toofar =true;
                    //    break;
                    //}
                }

            }
            if(goodenough || toofar)
                break;
        }
        if((bracketscount > 0 || toofar) && bracketstarts.size() > 0)
        {
            //display error
            QTextCharFormat format;
            format = keywordFormat;

            if(toofar)
            {
                format.setForeground( QColor::fromRgbF(1.0f,1.0f,0.1f,1.0f));
                setFormat(bracketstarts.front(), 1, format);
            }
            else
            {
                format.setForeground( QColor::fromRgbF(1.0f,0.0f,0.0f,1.0f));
                setFormat(bracketstarts.front(), 1, format);
            }
        }
    }
    bool rehighlightNext = false;
    // Special pass, for errors like no commas before keywords
    for(int i = 0; i < tokens[bn].size();i++)
    {
        bool markPrevAsError = false;
        bool markAsError = false;
        bool grabbedPrevline = false;
        QString prevToken = "";
        int prevstart = -1;
        int prevend = -1;
        int prevBnID = 1;
        if (i > 0)
        {
            prevToken = tokens[bn][i-1].text;
            prevstart = tokens[bn][i-1].start;
            prevend = tokens[bn][i-1].end;
        }
        else while (bn-prevBnID >= 0 && prevToken.size()<=0 )
        {
            if(tokens[bn-prevBnID].size() > 0 && !tokens[bn-prevBnID][0].text.contains("--"))
            {
                prevToken = tokens[bn-prevBnID].back().text;
                prevstart = tokens[bn-prevBnID].back().start;
                prevend = tokens[bn-prevBnID].back().end;
                grabbedPrevline = true;
            }
            else
                prevBnID++;
        }


        if(keywordPatterns.contains(tokens[bn][i].text,Qt::CaseInsensitive))
        {
            if((tokens[bn][i].text.size() > 1 && !isWindowFunc(tokens[bn][i].text) && (prevToken.endsWith('.') || prevToken.endsWith(',')))
                ||  (tokens[bn][i].text.toLower() == "select" && prevToken.size() > 0 && !prevToken.contains(';')&& !prevToken.contains("all")&& !prevToken.contains("union") && !prevToken.contains(')') && !prevToken.contains('('))
                //||  ((tokens[bn][i].text.toLower() == "and" || tokens[bn][i].text.toLower() == "or") && prevToken.size() > 1  && keywordPatterns.contains(prevToken,Qt::CaseInsensitive))
                )
            {
                markPrevAsError  = true;
                markAsError = true;
                if(grabbedPrevline)
                    rehighlightNext = true;
            }
        }
        if((tokens[bn][i].text.startsWith(',') && tokens[bn][i].text.endsWith(')'))
            || (prevToken.endsWith(',') && tokens[bn][i].text.startsWith(')')) )
        {
            markPrevAsError  = true;
            markAsError = true;
            if(grabbedPrevline)
                rehighlightNext = true;
        }
        if(markAsError)
        {
            QTextCharFormat format;
            format = keywordFormat;
            format.setForeground( QColor::fromRgbF(1.0f,0.0f,0.0f,1.0f));
            setFormat(tokens[bn][i].start,tokens[bn][i].text.size(), format);
            if(markPrevAsError  && prevend>=0 && prevstart >=0 )setFormat(prevstart,prevToken.size(), format);
            //qDebug()<<"typo: coma before from";
        }
    }

    //if(rehighlightNext)
    //    rehighlightBlock(currentBlock().next());
}

void Highlighter::OnBlockCountChanged(int newBlockCount)
{
    prevBlockCount = currentBlockCount;
    currentBlockCount = newBlockCount;
    BlockCountDiff = currentBlockCount - prevBlockCount;
    newBlockCount = true;
    qDebug()<< "highlighter new block count " << newBlockCount;
    qDebug()<< "current block " << currentBlockCount << "   /    " << prevBlockCount << "   diff = " << BlockCountDiff;
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
    dbSchemaName = dbname;

    if(PostgresStyle)
        keywordPatterns = PostgrePatterns;
    else if(QSLiteStyle)
        keywordPatterns = SQLitePatterns;
    else
        keywordPatterns = OraclePatterns;

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
