#include "highlighter.h"
#include "Patterns.h"
#include <qdir.h>
#include <qsqlerror.h>
#include <qsqlquery.h>
#include <qsqlrecord.h>
#include <QSqlDriver>
#include "datastorage.h"
#include "sqlSubfunctions.h"
inline DataStorage userDS;
inline QString documentsDir;

/* fix subtables
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

    updateMisc();
}



void Highlighter::highlightBlock(const QString &text)
{


    // CustomBlockData* userData = (CustomBlockData*)currentBlock().userData();
    // if(userData == nullptr)
    //     userData = new CustomBlockData();
    int bn = currentBlock().blockNumber();

    // if(!updateAllHighlighting && userData->lastText == text && lineInterval.size() > 0 && lineInterval[bn].size()>0 && lineIntervalFormats.size() == lineInterval.size() && lineIntervalFormats[bn].size() == lineInterval[bn].size())
    // {
    //     for(int i=0;i< lineInterval[bn].size();i++)
    //         setFormat(lineInterval[bn][i].first,lineInterval[bn][i].second, lineIntervalFormats[bn][i]);

    //     return;
    // }
    // currentBlock().setUserData(userData);



    std::vector<int> tabcount; // count amount of tabs for before each character




    while(lineInterval.size() <= bn)
    {
        lineInterval.emplace_back();
        lineIntervalColor.emplace_back();
        lineIntervalFormats.emplace_back();
    }


    lineInterval[bn].clear();
    lineIntervalColor[bn].clear();
    lineIntervalFormats[bn].clear();

    tokens[bn].clear();
    QStringList words = text.split(' ');
    int wordstart = 0;
    int wordend = 0;
    bool inword = false;
    QString word = "";

    tabcount.resize(text.size());
    int i_tabcnt = 0;
    // split, Cant use one sqlSubfunction.h due to tokens beeng a TextInterval
    for(int i =0;i<text.size();i++)
    {
        if(text[i] == '\t')
            i_tabcnt++;
        tabcount[i] = i_tabcnt;
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
            inword = false;
            i++;// hop to the next unprocessed symbol
        }
        else if(isSpecialSymbol(text[i])) // only this
        {
            QString tword = text[i];
            tokens[currentBlock().blockNumber()].push_back( {i, i, tword} );
            word.clear();
            inword = false;
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


    // simple aliases in join statements
    TableAliasMapPerRow[bn].clear();
    for (int i = 2 ; i < tokens[bn].size() ; i++)
    {

        QString Prevword = tokens[bn][i-1].text;
        if(Prevword.endsWith('"'))
            Prevword = tokens[bn][i-2].text;

        QString word = tokens[bn][i].text;
        if(!PostgresStyle)
        {
            word = word.toLower();
            Prevword  = Prevword.toLower();
        }

        if (TableColumnMap.contains(Prevword ) &&
            !keywordPatterns.contains(word ,Qt::CaseInsensitive))
        {
            TextInterval tmpti;
            tmpti.start = tokens[bn][i].start;
            tmpti.end = tokens[bn][i].end;
            tmpti.text = tokens[bn][i].text;
            tmpti.PrevWord = Prevword;
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

    // complex aliases, like with table_name as (select column ,..... from
    // or (select column,column, from ...) tablename
    tmpTableColumnMap.clear();
    tmpTableColumnMapPerRow[bn].clear();
    for(int i=0;i < tokens[bn].size();i++)
    {
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
            SteppedWord = SteppedWord.toLower();
            prevword = prevword.toLower();
        }
        if(!keywordPatterns.contains(word,Qt::CaseInsensitive))
        {// probably reached a "something something ) tableAlias"
            QString tableAlias = word;
            int a = i - 1;
            int blocknum = bn;
            int prevA = i - 1;
            int prevBlocknum = bn;
            int bracket_count = 0;
            bool isSelect = false;
            bool forceExit = false;
            if(SteppedWord.endsWith(")"))
            {
                while (a >= 0 && blocknum >= 0)
                {// step left
                    bracket_count += tokens[blocknum][a].text.count(")") - tokens[blocknum][a].text.count("(");

                    if(bracket_count<=0)
                        break;

                    prevA = a;
                    prevBlocknum = blocknum;

                    a -= 1;
                    while (a < 0)
                    {
                        blocknum -= 1;
                        if(blocknum >= 0 && tokens[blocknum].size() > 0)
                            a = tokens[blocknum].size()-1;
                        else if(blocknum < 0)
                            break;
                    }
                }
                // found a line of start of this possibly inner query
                a = prevA;
                blocknum = prevBlocknum ;

                if(subCommandPatterns.contains(tokens[blocknum][a].text.toLower().trimmed(),Qt::CaseInsensitive))
                {// its probably some subcommand, like subexectounionalltable or something, so we skip
                    qDebug()<< "highlighter: " << tokens[blocknum][a].text << "  is a subcomand, moving around " << blocknum << a;
                    a++;
                    while(blocknum < tokens.size() && (a >= tokens[blocknum].size() ||tokens[blocknum][a].text.startsWith('{') ) ||
                           (blocknum < tokens.size() && tokens[blocknum].size()>0 && tokens[blocknum][0].text.startsWith("--")))
                    { // move to next valid token, needs to be bracket
                        blocknum++;
                        a = 0;
                    }
                    qDebug()<< "highlighter: " << tokens[blocknum][a].text << "  should be select " << blocknum << a;

                }
                // this word must be select, otherwise it isnt a select statment, thus its not an alias
                if(tokens[blocknum][a].text.toLower().trimmed() == "select")
                {
                    qDebug() <<word << " is subtable style alias";
                    isSelect = true;

                }
            }
            else
            { // second try, now if its something like "with Tablename as ( select asdasdas,asdasd,asdasd from..."

                // reset
                QString tableAlias = word;
                a = i + 1;
                blocknum = bn;
                prevA = i + 1;
                prevBlocknum = bn;
                bracket_count = 0;

                while(blocknum < tokens.size() && a >= tokens[blocknum].size())
                { // move to next valid token
                    blocknum++;
                    a = 0;
                }
                if(blocknum >= tokens.size())
                    forceExit = true;

                if(!forceExit)
                if (tokens[blocknum][a].text.toLower().trimmed() == "as")
                {
                    a++;
                    while(blocknum < tokens.size() && a >= tokens[blocknum].size())
                    { // move to next valid token, needs to be bracket
                        blocknum++;
                        a = 0;
                    }
                    if(blocknum >= tokens.size())
                        forceExit = true;

                    if(!forceExit)
                    if (tokens[blocknum][a].text.toLower().trimmed().startsWith("("))
                    {
                        a++;
                        while(blocknum < tokens.size() && a >= tokens[blocknum].size())
                        { // move to next valid token, now it should be select
                            blocknum++;
                            a = 0;
                        }
                        if(blocknum >= tokens.size())
                            forceExit = true;
                        if(!forceExit)
                        if (tokens[blocknum][a].text.toLower().trimmed() == "select")
                        {
                            qDebug() <<word << " is 'with' style alias";
                            isSelect = true;
                        }
                    }
                }
            }

            if(blocknum >= tokens.size())
                forceExit = true;
            if(a >= tokens[blocknum].size())
                forceExit = true;
            if(!forceExit)
            {

                if(isSelect)
                {
                    QString prevtoken = "";
                    QStringList aliasColumns;
                    int iter = 0;
                    bracket_count = 0;

                    while(true)
                    {
                        iter ++;

                        a++;
                        if(a>=tokens[blocknum].size())
                        {
                            blocknum ++;
                            a = 0;
                        }

                        // find next not empty row of tokens
                        while(blocknum < tokens.size() && tokens[blocknum].size() <=0)
                        {
                            blocknum++;
                            // reset to 0
                            a = 0;
                        }
                        // quit if reached end
                        if(blocknum >= tokens.size() || a >= tokens[blocknum].size())
                            break;



                        // if comment, skip to next line
                        if(tokens[blocknum][a].text.contains("--") || (a > 0 && tokens[blocknum][a].text.startsWith('-') && tokens[blocknum][a-1].text.endsWith('-') && tokens[blocknum][a-1].end +1 == tokens[blocknum][a].start))
                        {
                            a = 0;
                            blocknum++;
                            continue;
                        }

                        // before , or before from -> column name;
                        if((tokens[blocknum][a].text.startsWith(",") || tokens[blocknum][a].text.toLower() == "from" ) && bracket_count == 0 && prevtoken.size()>0)
                        {
                            aliasColumns.push_back(prevtoken);
                        }
                        bracket_count -= tokens[blocknum][a].text.toLower().count(")");
                        bracket_count += tokens[blocknum][a].text.toLower().count("(");
                        if(tokens[blocknum][a].text.toLower() == "from" || bracket_count <0)
                            break;
                        prevtoken = tokens[blocknum][a].text.toLower();

                    }
                    for(auto x : tmpTableColumnMap[tableAlias].keys())
                    { // delete previous
                        tmpTableColumnMapPerRow[bn][tableAlias][x] = false;
                    } // add new columns
                    for(auto s : aliasColumns)
                    {
                        tmpTableColumnMapPerRow[bn][tableAlias][s] = true;
                    }
                }
            }
        }
    }

    for(auto ta : tmpTableColumnMapPerRow)
    {
        for(auto i : ta.keys())
        {
            for(auto x : ta[i].keys())
            {
                tmpTableColumnMap[i][x] = ta[i][x];
            }
        }
    }

    // basic token highlighting, depending on what it is
    for(int i=0;i < tokens[bn].size();i++)
    {
        if(tokens[bn][i].start<0 || tokens[bn][i].end < 0)
            continue;
        QTextCharFormat format;
        format.setForeground( QColor::fromRgbF(0.95f,0.95f,0.85f,1.0f));


        QString word = tokens[bn][i].text;
        QString prevword = "";
        if(i>0)
            prevword = tokens[bn][i-1].text;
        QString SteppedWord = "";
        if(i - 2 >= 0)
            SteppedWord = tokens[bn][i-2].text;
        if(!PostgresStyle)
        {
            word = word.toLower();
            SteppedWord = SteppedWord .toLower();
            prevword = prevword.toLower();
        }
        if(/*is word*/ isWord(word))
        {
            bool addQuotesToToken = false;
            // keywords
            if(keywordPatterns.contains(word,Qt::CaseInsensitive) && !prevword.contains('.'))
            {
                format = keywordFormat;
            }



            // columns
            if(ColumnMap_lower.contains(word.toLower()))
            {
                format = NameFormat;
                addQuotesToToken = true;
            }
            // columns of tables
            if(TableColumnMap.contains(SteppedWord ))
            {if(TableColumnMap[SteppedWord].contains(word))
                {
                    format = NameFormat;
                    addQuotesToToken = true;
                }}
            // columns of aliases
            if(TableColumnAliasMap.contains(SteppedWord))
            {if(TableColumnMap[TableColumnAliasMap[SteppedWord]].contains(word))
                {
                    format = NameFormat;
                    addQuotesToToken = true;
                }}
            // columns of complex aliases
            if(tmpTableColumnMap.contains(SteppedWord ))
            {if(tmpTableColumnMap[SteppedWord].contains(word))
                {
                    format = NameFormat;
                    addQuotesToToken = true;
                }}

            // repeat with prevword, not stepped
            // columns of tables
            if(TableColumnMap.contains(prevword ))
            {if(TableColumnMap[prevword].contains(word))
                {
                    format = NameFormat;
                    addQuotesToToken = true;
                }}
            // columns of aliases
            if(TableColumnAliasMap.contains(prevword))
            {if(TableColumnMap[TableColumnAliasMap[prevword]].contains(word))
                {
                    format = NameFormat;
                    addQuotesToToken = true;
                }}
            // columns of complex aliases
            if(tmpTableColumnMap.contains(prevword ))
            {if(tmpTableColumnMap[prevword].contains(word))
                {
                    format = NameFormat;
                    addQuotesToToken = true;
                }}


            // tables, aliases, complex aliases
            if((TableColumnMap.contains(word) || TableColumnAliasMap.contains(word) || tmpTableColumnMap.contains(word)))
            {
                format = classFormat;
                addQuotesToToken = true;
            }
            if(isNumber(word))
            {
                format = keywordFormat;
                format.setForeground( QColor::fromRgbF(0.45f,0.65f,0.85f,1.0f));
                addQuotesToToken = false;
            }
            if(subCommandPatterns.contains(word,Qt::CaseInsensitive))
            {
                format = keywordFormat;
                format.setForeground(QColor::fromRgbF(0.7f,0.7f,0.2f,1.0f));
                addQuotesToToken = false;
            }

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



        }
        else if(word.size() == 1 && isSpecialSymbol(word[0]))
            format = keywordFormat;
        else if(word.size() == 2 && isSpecialSymbol(word[0]) && isSpecialSymbol(word[1]))
            format = keywordFormat;
        setFormat(tokens[bn][i].start,tokens[bn][i].end - tokens[bn][i].start+1, format);

        lineInterval[bn].push_back({tokens[bn][i].start + tabcount[tokens[bn][i].start]*4,tokens[bn][i].end - tokens[bn][i].start+1});
        lineIntervalColor[bn].push_back(format.foreground().color());
        lineIntervalFormats[bn].push_back(format);

    }

    bool quote = false;
    int quote_start = 0;

    int bracketscount = 0;
    std::vector <int> bracketstarts;
    bool comment = false;

    //positional highlighting, brackets, comments, quotes.
    for(int i = 0;i < tokens[bn].size();i++)
    {
        if(tokens[bn][i].text.contains("--") || (i > 0 && tokens[bn][i].text.startsWith('-') && tokens[bn][i-1].text.endsWith('-') && tokens[bn][i-1].end +1 == tokens[bn][i].start))
            comment = true;
        if(comment)
        {
            setFormat(tokens[bn][i].start,text.size(), multiLineCommentFormat);

            lineInterval[bn].push_back({tokens[bn][i].start + tabcount[tokens[bn][i].start]*4,text.size() - tokens[bn][i].start});
            lineIntervalColor[bn].push_back(multiLineCommentFormat.foreground().color());
            lineIntervalFormats[bn].push_back(multiLineCommentFormat);

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

                lineInterval[bn].push_back({quote_start + tabcount[quote_start]*4,(tokens[bn][i].start + 1) - quote_start});
                lineIntervalColor[bn].push_back(quotationFormat.foreground().color());
                lineIntervalFormats[bn].push_back(quotationFormat);
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

    // Special highlighting pass, for errors like no commas before 'from' and other simple errors
    for(int i = 0; i < tokens[bn].size();i++)
    {
        bool markPrevAsError = false;
        bool markNextAsError  = true;
        bool markAsError = false;
        bool grabbedPrevline = false;
        bool grabbedNextline = false;
        QString prevToken = "";
        int     prevstart = -1;
        int     prevend = -1;
        int     prevBnID = 1;
        QString nextToken = "";
        int     nextstart = -1;
        int     nextend = -1;
        int     nextBnID = 1;
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

        if (i < tokens[bn].size() - 1) // if next exists in this line
        {
            nextToken = tokens[bn][i + 1].text;
            nextstart = tokens[bn][i + 1].start;
            nextend = tokens[bn][i + 1].end;

        }
        else
            while ( bn + nextBnID < tokens.size() && nextToken.size()<=0 ) // if next nonempty exists
            {
                if(tokens[bn+nextBnID].size() > 0 && !tokens[bn+nextBnID][0].text.contains("--")) // non empty non coment
                {
                    nextToken = tokens[bn + nextBnID].front().text;
                    nextstart = tokens[bn + nextBnID].front().start;
                    nextend = tokens[bn +  nextBnID].front().end;
                    grabbedNextline = true;
                }
                else
                    nextBnID++;
            }


        if(keywordPatterns.contains(tokens[bn][i].text,Qt::CaseInsensitive))
        {
            if( (tokens[bn][i].text.toLower() == "select" && prevToken.size() > 0 && !prevToken.contains(';')&& !prevToken.contains('{')&& !prevToken.contains('}')&& !prevToken.contains("all")&& !prevToken.contains("union") && !prevToken.contains(')') && !prevToken.contains('('))
                )
            {
                markPrevAsError  = true;
                markAsError = true;
                markNextAsError = false;
                if(grabbedPrevline)
                    rehighlightNext = true;
            }
        }

        if((tokens[bn][i].text.startsWith(',') && tokens[bn][i].text.endsWith(')'))
            || (prevToken.endsWith(',') && tokens[bn][i].text.startsWith(')')) )
        {
            markPrevAsError  = true;
            markAsError = true;
            markNextAsError = false;
            if(grabbedPrevline)
                rehighlightNext = true;
        }

        if((tokens[bn][i].text.toLower() == "and" || tokens[bn][i].text.toLower() == "or" ) &&
            (prevToken.toLower() == "and" || prevToken.toLower() == "or" || prevToken.toLower() == "where" || prevToken.toLower() == "between") )
        {
            markAsError = true;
            markPrevAsError  = true;
            markNextAsError = false;
        }

        if((tokens[bn][i].text.toLower() == "and" || tokens[bn][i].text.toLower() == "or" ) &&
            (nextToken.toLower() == "and" || nextToken.toLower() == "or" || nextToken.toLower() == "where" || nextToken.toLower() == "between") )
        {
            markAsError = true;
            markPrevAsError  = false;
            markNextAsError = true;
        }

        if(tokens[bn][0].text != "--" && tokens[bn][i].text != "--" && isSpecialSymbol(tokens[bn][i].text.back())
            && tokens[bn][i].text != "type" // way too frequent
            && !(tokens[bn][i].text.endsWith('\'') ||tokens[bn][i].text.endsWith(')') || tokens[bn][i].text.endsWith('(') || tokens[bn][i].text.endsWith('*')|| tokens[bn][i].text.endsWith(';') || tokens[bn][i].text.endsWith('"'))
            && !TableColumnMap_lower.contains(nextToken) && !ColumnMap_lower.contains(nextToken) && nextToken.size() > 1 &&
               (nextToken == "and" ||
                nextToken == "or" ||
                nextToken == "where" ||
                nextToken == "select" ||
                nextToken == "from" ||
                nextToken == "where" ||
                nextToken == "group" ||
                nextToken == "order"
                ))
        {
            markAsError = true;
            markPrevAsError  = false;
            markNextAsError = true;
        }

        if(markAsError)
        {
            QTextCharFormat format;
            format = keywordFormat;
            format.setForeground( QColor::fromRgbF(1.0f,0.0f,0.0f,1.0f));
            setFormat(tokens[bn][i].start,tokens[bn][i].text.size(), format);
            if(markPrevAsError  && prevend>=0 && prevstart >=0 )setFormat(prevstart,prevToken.size(), format);
            if(markNextAsError  && nextend>=0 && nextstart >=0 )setFormat(nextstart,nextToken.size(), format);

            lineInterval[bn].push_back({tokens[bn][i].start  + tabcount[tokens[bn][i].start]*4,tokens[bn][i].text.size()});
            lineIntervalColor[bn].push_back(QColor::fromRgbF(1.0f,0.0f,0.0f,1.0f));
            lineIntervalFormats[bn].push_back(format);


        }
    }

    // userData = (CustomBlockData*)currentBlock().userData();
    // if(userData == nullptr)
    //     userData = new CustomBlockData();

    // userData->lastText = text;

    // currentBlock().setUserData(userData);

}


void Highlighter::updateMisc()
{
    // load user theme



    if(userDS.data.count("UserTheme") > 0)
    {

        int fnt_size = FontSizeOverride;
        if(fnt_size  == -1) fnt_size = userDS.GetPropertyAsInt("UserTheme","FontSize");


        if(userDS.data["UserTheme"]["Italic_KeyWord"].trimmed().toLower() == "true")
            keywordFormat.setFontItalic(true);
        else
            keywordFormat.setFontItalic(false);
        if(userDS.data["UserTheme"]["Bold_KeyWord"].trimmed().toLower() == "true")
            keywordFormat.setFontWeight(QFont::Bold);
        else
            keywordFormat.setFontWeight(QFont::Weight::Normal);



        //strl = QString(userDS.data["UserTheme"]["Color_KeyWord"]).split(',');
        QColor col = QColor::fromRgbF(0.25f,0.55f,0.95f,1.0f);

        for(int i=0;i<QString(userDS.data["UserTheme"]["Color_KeyWord"]).split(',').size();i++)
        {
            if(i==0) col.setRed(QVariant(QString(userDS.data["UserTheme"]["Color_KeyWord"]).split(',')[i]).toInt());
            if(i==1) col.setGreen(QVariant(QString(userDS.data["UserTheme"]["Color_KeyWord"]).split(',')[i]).toInt());
            if(i==2) col.setBlue(QVariant(QString(userDS.data["UserTheme"]["Color_KeyWord"]).split(',')[i]).toInt());
            if(i==3) col.setAlpha(QVariant(QString(userDS.data["UserTheme"]["Color_KeyWord"]).split(',')[i]).toInt());
        }

        keywordFormat.setForeground( col);
        keywordFormat.setFontPointSize(fnt_size);
        if(userDS.data["UserTheme"]["Font"].size()>2)
            keywordFormat.setFontFamilies(QStringList(QString(userDS.data["UserTheme"]["Font"]).trimmed()));






        if(QString(userDS.data["UserTheme"]["Italic_column_name"]).trimmed().toLower() == "true") // columns
            NameFormat.setFontItalic(true);
        else
            NameFormat.setFontItalic(false);
        if(QString(userDS.data["UserTheme"]["Bold_column_name"]).trimmed().toLower() == "true")
            NameFormat.setFontWeight(QFont::Bold);
        else
            NameFormat.setFontWeight(QFont::Weight::Normal);

        //strl2 = QString(userDS.data["UserTheme"]["Color_column_name"]).split(',');
        col = QColor::fromRgbF(0.85f,0.45f,0.15f,1.0f);
        for(int i=0;i<QString(userDS.data["UserTheme"]["Color_column_name"]).split(',').size();i++)
        {
            if(i==0) col.setRed(QVariant(QString(userDS.data["UserTheme"]["Color_column_name"]).split(',')[i]).toInt());
            if(i==1) col.setGreen(QVariant(QString(userDS.data["UserTheme"]["Color_column_name"]).split(',')[i]).toInt());
            if(i==2) col.setBlue(QVariant(QString(userDS.data["UserTheme"]["Color_column_name"]).split(',')[i]).toInt());
            if(i==3) col.setAlpha(QVariant(QString(userDS.data["UserTheme"]["Color_column_name"]).split(',')[i]).toInt());
        }
        NameFormat.setForeground( col );
        NameFormat.setFontPointSize(fnt_size);
        if(userDS.data["UserTheme"]["Font"].size()>2)
            NameFormat.setFontFamilies(QStringList(QString(userDS.data["UserTheme"]["Font"]).trimmed()));




        if(QString(userDS.data["UserTheme"]["Italic_table_name"]).trimmed().toLower() == "true") // tables
            classFormat.setFontItalic(true);
        else
            classFormat.setFontItalic(false);
        if(QString(userDS.data["UserTheme"]["Bold_table_name"]).trimmed().toLower() == "true")
            classFormat.setFontWeight(QFont::Bold);
        else
            classFormat.setFontWeight(QFont::Weight::Normal);
        // strl3 = QString(userDS.data["UserTheme"]["Color_table_name"]).split(',');
        col = QColor::fromRgbF(0.65f,0.25f,0.65f,1.0f);
        for(int i=0;i<QString(userDS.data["UserTheme"]["Color_table_name"]).split(',').size();i++)
        {
            if(i==0) col.setRed(QVariant(QString(userDS.data["UserTheme"]["Color_table_name"]).split(',')[i]).toInt());
            if(i==1) col.setGreen(QVariant(QString(userDS.data["UserTheme"]["Color_table_name"]).split(',')[i]).toInt());
            if(i==2) col.setBlue(QVariant(QString(userDS.data["UserTheme"]["Color_table_name"]).split(',')[i]).toInt());
            if(i==3) col.setAlpha(QVariant(QString(userDS.data["UserTheme"]["Color_table_name"]).split(',')[i]).toInt());
        }
        classFormat.setForeground( col );
        classFormat.setFontPointSize(fnt_size);
        if(userDS.data["UserTheme"]["Font"].size()>2)
            classFormat.setFontFamilies(QStringList(QString(userDS.data["UserTheme"]["Font"]).trimmed()));




        if(QString(userDS.data["UserTheme"]["Italic_quotation"]).trimmed().toLower() == "true")
            quotationFormat.setFontItalic(true);
        else
            quotationFormat.setFontItalic(false);
        if(QString(userDS.data["UserTheme"]["Bold_quotation"]).trimmed().toLower() == "true")
            quotationFormat.setFontWeight(QFont::Weight::Bold);
        else
            quotationFormat.setFontWeight(QFont::Weight::Normal);

        //strl4 = QString(userDS.data["UserTheme"]["Color_quotation"]).split(',');
        col = QColor::fromRgbF(0.6f,0.5f,0.25f,1.0f);
        for(int i=0;i<QString(userDS.data["UserTheme"]["Color_quotation"]).split(',').size();i++)
        {
            if(i==0) col.setRed(QVariant(QString(userDS.data["UserTheme"]["Color_quotation"]).split(',')[i]).toInt());
            if(i==1) col.setGreen(QVariant(QString(userDS.data["UserTheme"]["Color_quotation"]).split(',')[i]).toInt());
            if(i==2) col.setBlue(QVariant(QString(userDS.data["UserTheme"]["Color_quotation"]).split(',')[i]).toInt());
            if(i==3) col.setAlpha(QVariant(QString(userDS.data["UserTheme"]["Color_quotation"]).split(',')[i]).toInt());
        }
        quotationFormat.setForeground(col);
        quotationFormat.setFontPointSize(fnt_size);
        if(userDS.data["UserTheme"]["Font"].size()>2)
            quotationFormat.setFontFamilies(QStringList(QString(userDS.data["UserTheme"]["Font"]).trimmed()));




        if(QString(userDS.data["UserTheme"]["Italic_comment"]).trimmed().toLower() == "true")
            multiLineCommentFormat.setFontItalic(true);
        else
            multiLineCommentFormat.setFontItalic(false);

        if(QString(userDS.data["UserTheme"]["Bold_comment"]).trimmed().toLower() == "true")
            multiLineCommentFormat.setFontWeight(QFont::Bold);
        else
            multiLineCommentFormat.setFontWeight(QFont::Weight::Normal);
        // strl5 = QString(userDS.data["UserTheme"]["Color_comment"]).split(',');
        col = QColor::fromRgbF(0.15f,0.75f,0.15f,1.0f);
        for(int i=0;i<QString(userDS.data["UserTheme"]["Color_comment"]).split(',').size();i++)
        {
            if(i==0) col.setRed(QVariant(QString(userDS.data["UserTheme"]["Color_comment"]).split(',')[i]).toInt());
            if(i==1) col.setGreen(QVariant(QString(userDS.data["UserTheme"]["Color_comment"]).split(',')[i]).toInt());
            if(i==2) col.setBlue(QVariant(QString(userDS.data["UserTheme"]["Color_comment"]).split(',')[i]).toInt());
            if(i==3) col.setAlpha(QVariant(QString(userDS.data["UserTheme"]["Color_comment"]).split(',')[i]).toInt());
        }
        multiLineCommentFormat.setForeground(col);
        multiLineCommentFormat.setFontPointSize(fnt_size);
        if(userDS.data["UserTheme"]["Font"].size()>2)
            multiLineCommentFormat.setFontFamilies(QStringList(QString(userDS.data["UserTheme"]["Font"]).trimmed()));



        //Courier NEW

        functionFormat.setForeground(Qt::blue);


    }
    else
    {
        // if cant open file, set stock theme
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

    qDebug() << "highlighter updated";

    singleLineCommentFormat.setForeground(QColor(Qt::green).lighter(90));
}

void Highlighter::OnBlockCountChanged(int newBlockCount)
{
    // attempt to rehighlight only whats needed
    prevBlockCount = currentBlockCount;
    currentBlockCount = newBlockCount;
    BlockCountDiff = currentBlockCount - prevBlockCount;
    newBlockCount = true;
    qDebug()<< "highlighter new block count " << newBlockCount;
    qDebug()<< "current block " << currentBlockCount << "   /    " << prevBlockCount << "   diff = " << BlockCountDiff;
}

void Highlighter::UpdateTableColumns(QSqlDatabase* db, QString dbname)
{// update data about tables, columns and dbPatterns

    ColumnMap.clear();
    TableAliasMapPerRow.clear();
    TableColumnAliasMap.clear();
    TableColumnMap.clear();

    TableColumnAliasMap_lower.clear();
    TableColumnMap_lower.clear();

    dbPatterns.clear();
    TableColumnDS.data.clear();
    AllTableColumnDS.data.clear();
    dbSchemaName = dbname;

    if(PostgresStyle)
        keywordPatterns = PostgrePatterns;
    else if(QSLiteStyle)
        keywordPatterns = SQLitePatterns;
    else
        keywordPatterns = OraclePatterns;

    // if not sqlite, load using special query, save as schemaName.txt
    if(!QSLiteStyle)
    {
        //strl = db->databaseName().split(';');
        QString filename =documentsDir + "/"+ dbname + QString(".txt");
        if(QFile::exists(filename))
        {
            TableColumnDS.Load(filename);
        }


        if(QFile::exists(documentsDir + "/All_"+ dbname + QString(".txt")))
        {
            AllTableColumnDS.Load(documentsDir + "/All_"+ dbname + QString(".txt"));
        }

    }
    else
    {// if sqlite, load directly from it

        db->open();

        for(auto tbl : db->driver()->tables(QSql::AllTables))
        {
            TableColumnDS.data[tbl.toLower()];
        }
        for(auto tbn : db->driver()->tables(QSql::AllTables))
        {
            QSqlRecord rec = db->driver()->record(tbn);
            int reccount = rec.count();
            for(int a=0,total = reccount; a<total;a++)
            {
                TableColumnDS.data[tbn.toLower()][rec.fieldName(a).toLower()];
            }
        }

    }

    for(auto x : TableColumnDS.data)
    {
        for(auto y : x.second)
        {
            QString word = y.first.trimmed() +' '+ y.second;
            if(y.second.trimmed().size()<=0)
                word = y.first;
            word = word.trimmed();



            TableColumnMap[x.first][word] = true;
            TableColumnMap_lower[x.first.toLower()][word.toLower()] = true;
            ColumnMap[word] = true;
            ColumnMap_lower[word.toLower()] = true;

        }
        dbPatterns.push_back(x.first);
    }
}
