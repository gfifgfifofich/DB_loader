#include "codeeditor.h"
#include "datastorage.h"
#include "linenumberarea.h"
#include <QPainter>
#include <QTextBlock>
#include <QFile>
#include <qmenu.h>
#include <qshortcut.h>
#include <QTextCursor>
#include <QPaintDevice>
#include "Patterns.h"
#include "highlighter.h"
#include <QFont>
#include <QtGui>
#include "sqlSubfunctions.h"
#include "tokenprocessor.h"
#include <math.h>


inline QString usrDir;
inline QString documentsDir;
inline DataStorage userDS;
inline QColor braccketHighlightColor = QColor(Qt::GlobalColor::darkRed).lighter(35);


void CodeEditor::keyPressEvent(QKeyEvent *e)
{
    //16777217 tab
    //16777220 return
    //16777248 SHIFT
    //16777249 ctrl
    //qDebug()<< e->key();
    if(e->key() == 16777217)
    {
        FillsuggestName();
        return;
    }
    QPlainTextEdit::keyPressEvent(e);
}

CodeEditor::CodeEditor(QWidget *parent) : QPlainTextEdit(parent)
{

    setTabStopDistance(QFontMetricsF(this->font()).horizontalAdvance(' ') * 4);

    lineNumberArea = new LineNumberArea(this);
    highlighter = new Highlighter(document());

    updateLineNumberAreaWidth(0);
    highlightCurrentLine();

    QFont font;
    font.setFamily("Courier");
    font.setFixedPitch(true);
    font.setPointSize(10);
    setFont(font);



    connect(this,&CodeEditor::cursorPositionChanged,this,&CodeEditor::suggestName, Qt::QueuedConnection );
    connect(this, &CodeEditor::blockCountChanged, this, &CodeEditor::updateLineNumberAreaWidth);
    connect(this, &CodeEditor::updateRequest, this, &CodeEditor::updateLineNumberArea);
    connect(this, &CodeEditor::cursorPositionChanged, this, &CodeEditor::highlightCurrentLine);

    // autocompleation on tab press
    connect( this, SIGNAL(tabDetected()), this, SLOT(fillName()), Qt::QueuedConnection );

    // user theme
    if(userDS.Load((documentsDir + "/userdata.txt").toStdString()))
    {
        QStringList strl = QString(userDS.data["UserTheme"]["BracketHighlightColor"].c_str()).split(',');
        QColor col = QColor(Qt::GlobalColor::darkRed).lighter(35);
        for(int i=0;i<strl.size();i++)
        {
            if(i==0) col.setRed(QVariant(strl[i]).toInt());
            if(i==1) col.setGreen(QVariant(strl[i]).toInt());
            if(i==2) col.setBlue(QVariant(strl[i]).toInt());
            if(i==3) col.setAlpha(QVariant(strl[i]).toInt());
        }
        braccketHighlightColor = col;
    }
}

int CodeEditor::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }

    int space = 3 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;

    return space;
}
void CodeEditor::updateLineNumberAreaWidth(int  /*newBlockCount */)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}
void CodeEditor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
        lineNumberArea->scroll(0, dy);
    else
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}
void CodeEditor::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);

    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}
void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(lineNumberArea);
    painter.fillRect(event->rect(), Qt::black);
    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());
    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(Qt::white);
            painter.drawText(0, top, lineNumberArea->width(), fontMetrics().height(),
                             Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++blockNumber;
    }
}

void CodeEditor::highlightCurrentLine()
{
    QString text = toPlainText();
    int cursor_position = textCursor().position();

    bool validCursorPos = cursor_position >=0 && cursor_position <text.size();

    QList<QTextEdit::ExtraSelection> extraSelections;

    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;

        QColor lineColor = QColor(Qt::white).lighter(20);

        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }

    //always highlight brackets
    if(validCursorPos && (text[cursor_position] == '(' || (cursor_position-1 >0 && text[cursor_position-1] == '(')))
    {
        QTextEdit::ExtraSelection bracketSelection;

        int bracketStart = cursor_position;
        if(cursor_position-1 >0 && text[cursor_position-1] == '(')
            bracketStart = cursor_position-1;


        int bracketEnd = cursor_position;
        int bracketCount = 0;
        for (int i = bracketStart; i < text.size(); i++)
        {
            if(text[i]=='(')
                bracketCount ++;
            if(text[i]==')')
                bracketCount --;
            if(bracketCount <=0)
            {
                bracketEnd=i;
                break;
            }
            if (i ==  text.size()-1)
            {
                bracketEnd=i;
                break;
            }
        }

        QColor lineColor = braccketHighlightColor ;
        bracketSelection.format.setBackground(lineColor.lighter());
        bracketSelection.cursor = textCursor();
        bracketSelection.cursor.clearSelection();
        bracketSelection.cursor.setPosition(bracketStart, QTextCursor::MoveAnchor);
        bracketSelection.cursor.setPosition(bracketEnd+1, QTextCursor::KeepAnchor);

        extraSelections.append(bracketSelection);
    }
    else if(validCursorPos && (text[cursor_position] == ')' || (cursor_position-1 >0 && text[cursor_position-1] == ')')))
    {
        QTextEdit::ExtraSelection bracketSelection;

        int bracketStart = cursor_position;
        if(cursor_position-1 >0 && text[cursor_position-1] == ')')
            bracketStart = cursor_position-1;


        int bracketEnd = cursor_position;
        int bracketCount = 0;
        for (int i = bracketStart; i >= 0; i--)
        {
            if(text[i]=='(')
                bracketCount ++;
            if(text[i]==')')
                bracketCount --;
            if(bracketCount >=0)
            {
                bracketEnd=i;
                break;
            }
            if (i == 0)
            {
                bracketEnd=i;
                break;
            }
        }

        QColor lineColor = braccketHighlightColor ;
        bracketSelection.format.setBackground(lineColor.lighter());
        bracketSelection.cursor = textCursor();
        bracketSelection.cursor.clearSelection();
        bracketSelection.cursor.setPosition(bracketEnd, QTextCursor::MoveAnchor);
        bracketSelection.cursor.setPosition(bracketStart+1, QTextCursor::KeepAnchor);

        extraSelections.append(bracketSelection);
    }

    if(validCursorPos && (text[cursor_position] == '{' || (cursor_position-1 >0 && text[cursor_position-1] == '{')))
    {
        QTextEdit::ExtraSelection bracketSelection;

        int bracketStart = cursor_position;
        if(cursor_position-1 >0 && text[cursor_position-1] == '{')
            bracketStart = cursor_position-1;


        int bracketEnd = cursor_position;
        int bracketCount = 0;
        for (int i = bracketStart; i < text.size(); i++)
        {
            if(text[i]=='{')
                bracketCount ++;
            if(text[i]=='}')
                bracketCount --;
            if(bracketCount <=0)
            {
                bracketEnd=i;
                break;
            }
            if (i ==  text.size()-1)
            {
                bracketEnd=i;
                break;
            }
        }

        QColor lineColor = braccketHighlightColor ;
        bracketSelection.format.setBackground(lineColor.lighter());
        bracketSelection.cursor = textCursor();
        bracketSelection.cursor.clearSelection();
        bracketSelection.cursor.setPosition(bracketStart, QTextCursor::MoveAnchor);
        bracketSelection.cursor.setPosition(bracketEnd+1, QTextCursor::KeepAnchor);

        extraSelections.append(bracketSelection);
    }
    else if(validCursorPos && (text[cursor_position] == '}' || (cursor_position-1 >0 && text[cursor_position-1] == '}')))
    {
        QTextEdit::ExtraSelection bracketSelection;

        int bracketStart = cursor_position;
        if(cursor_position-1 >0 && text[cursor_position-1] == '}')
            bracketStart = cursor_position-1;


        int bracketEnd = cursor_position;
        int bracketCount = 0;
        for (int i = bracketStart; i >= 0; i--)
        {
            if(text[i]=='{')
                bracketCount ++;
            if(text[i]=='}')
                bracketCount --;
            if(bracketCount >=0)
            {
                bracketEnd=i;
                break;
            }
            if (i == 0)
            {
                bracketEnd=i;
                break;
            }
        }

        QColor lineColor = braccketHighlightColor ;
        bracketSelection.format.setBackground(lineColor.lighter());
        bracketSelection.cursor = textCursor();
        bracketSelection.cursor.clearSelection();
        bracketSelection.cursor.setPosition(bracketEnd, QTextCursor::MoveAnchor);
        bracketSelection.cursor.setPosition(bracketStart+1, QTextCursor::KeepAnchor);

        extraSelections.append(bracketSelection);
    }

    //if selected something, highlight it in text
    if(abs(textCursor().selectionEnd() - textCursor().selectionStart()) >= 1)
    {
        QTextEdit::ExtraSelection wordSelection;

        int bracketStart = cursor_position;
        if(cursor_position-1 >0 && text[cursor_position-1] == ')')
            bracketStart = cursor_position-1;


        QString selectedString = textCursor().selectedText();
        selectedString = selectedString.replace('?', '\n');
        qDebug() << "Selected: " << selectedString;
        int match = 0;
        int start = 0;
        for (int i = 0; i < text.size(); i++)
        {
            if (text[i] == selectedString[match])
            {
                if(match == 0)
                    start = i;
                match ++;
            }
            else
                match =0;

            if (match >= selectedString.size())
            {
                match =0;
                QTextEdit::ExtraSelection selection;
                selection.format.setBackground(QColor(Qt::white).lighter(25));
                selection.cursor = textCursor();
                selection.cursor.clearSelection();
                selection.cursor.setPosition(start, QTextCursor::MoveAnchor);
                selection.cursor.setPosition(i+1, QTextCursor::KeepAnchor);


                extraSelections.append(selection);
            }
        }

    }

    setExtraSelections(extraSelections);


}

void CodeEditor::replace(int _from, int _to, QString _what, QString _with)
{// better to reimplement to work from cursors, not just text replacing
    if(_from > _to)
        return;
    if(_what.size() < 1)
        return;

    QString text = toPlainText();

    QTextCursor cursor = textCursor();
    cursor.beginEditBlock();
    cursor.setPosition(1);

    int w_start = 0;
    int w_end = 0;
    int match = 0;
    int replace_count = 0;
    int sizeDiff =  _with.size() - _what.size();
    int lineNumber = 0;
    for(int i=0;i < text.size();i++)
    {
        if(text[i] == '\n')
            lineNumber++;
        if(lineNumber >= _from -1 && lineNumber < _to)
        {
            if(text[i] == _what[match])
            {
                if(match == 0)
                    w_start = i + (sizeDiff * replace_count);
                match++;
            }
            else
                match = 0;
            if(match == _what.size())
            {
                match = 0;
                w_end = i + (sizeDiff * replace_count);
                cursor.setPosition(w_start, QTextCursor::MoveAnchor);
                cursor.setPosition(w_end+1, QTextCursor::KeepAnchor);

                cursor.insertText(_with);
                replace_count++;
            }
        }
    }
    cursor.endEditBlock();

}
void CodeEditor::suggestName()
{

    QString word = "";
    QString PrevWord= "";
    QString PrevPrevWord= "";
    QString PrevPrevPrevWord= "";
    QStringList PrevWords;
    bool hasdot = false;
    int depth = 6; // amount of prevwords to check
    QString text = toPlainText();
    int curs = textCursor().position();
    if(curs>=1)
        curs-=1;
    int curscop = curs;
    int start = curs;
    int end = curs;

    // begining of over complicated token separation

    while(text[curscop].isLetter() ||text[curscop] == '_'||text[curscop] == '\"'||text[curscop] == '$' )
    {
        start = curscop;
        curscop-=1;
        if(curscop<0)
            break;
    }
    while(text[curs].isLetter() ||text[curs] == '_'|| text[curs] == '\"' ||text[curs] == '$')
    {
        end = curs;
        curs+=1;
        if(curs>text.size())
            break;
    }
    for(int i=start;i<=end;i++)
    {
        if(text[i]!='"')
            word+=text[i];
    }

    // finding interval that contains all related tokens
    int curs2 = start - 1;
    if(curs2>=1)
        curs2-=1;
    int curscop2 = curs2;
    int start2 = curs2;
    int end2 = curs2;
    for(int i =0;i < depth; i++)
    {
        curs2 = start2 - 1;
        if(curs2>=1)
            curs2-=1;
        curscop2 = curs2;
        start2 = curs2;
        end2 = curs2;
        while(text[curscop2].isLetter() ||text[curscop2] == '_'||text[curscop2] == '\"'||text[curscop2] == '$' )
        {
            start2 = curscop2;
            curscop2-=1;
            if(curscop2<0)
                break;
        }
        while(text[curs2].isLetter() ||text[curs2] == '_'||text[curs2] == '\"'||text[curs2] == '$')
        {
            end2 = curs2;
            curs2+=1;
            if(curs>text.size())
                break;
        }
        QString wrd = "";
        for(int i=start2;i<=end2;i++)
        {
            if(text[i]!='"')
                wrd+=text[i];
        }
    }



    word = word;
    PrevWord = "";
    PrevPrevWord = "";
    PrevPrevPrevWord = "";

    QString text_interval = "";
    for(int i=start2; i <= end && i < text.size();i++)
    {
        text_interval += text[i];
    }

    // Processing this interval into separate token list
    PrevWords.clear();
    QStringList text_interval_strl = processBlockToTokens(text_interval);
    bool onWhiteSpace = false;
    if(textCursor().position()-1 >=0 && (text[textCursor().position()-1] == ' ' || text[textCursor().position()-1] == '\t'|| text[textCursor().position()-1] == '\n'))
    {

        for(int i=0; i < text_interval_strl.size();i++)
        {
            PrevWords.push_back( text_interval_strl[i].replace('\n',' ').replace('\t',' ').replace('\u0000',' ').toLower().trimmed());
        }
        if(PrevWords.size() >= 3)
            PrevPrevPrevWord = PrevWords[PrevWords.size()-3].toLower();
        if(PrevWords.size() >= 2)
            PrevPrevWord = PrevWords[PrevWords.size()-2].toLower();
        if(PrevWords.size() >= 1)
            PrevWord = PrevWords[PrevWords.size()-1].toLower();
        word = "";
        onWhiteSpace = true;
        qDebug() << "On whitespace";
    }
    else
    {

        for(int i=0; i < text_interval_strl.size();i++)
        {
            PrevWords.push_back( text_interval_strl[i].replace('\n',' ').replace('\t',' ').replace('\u0000',' ').toLower().trimmed());
        }

        if(PrevWords.size() >= 4)
            PrevPrevPrevWord= PrevWords[PrevWords.size()-4].toLower();
        if(PrevWords.size() >= 3)
            PrevPrevWord = PrevWords[PrevWords.size()-3].toLower();
        if(PrevWords.size() >= 2)
            PrevWord = PrevWords[PrevWords.size()-2].toLower();
        if(PrevWords.size() >= 1)
            word = PrevWords[PrevWords.size()-1].toLower();
    }

    word = word.replace('\u0000',' ').trimmed().toLower();

    qDebug() << "word is " <<word << onWhiteSpace;
    qDebug() << "PrevWord is " <<PrevWord ;
    qDebug() << "PrevPrevWord is " <<PrevPrevWord ;
    qDebug() << "PrevPrevPrevWord is " <<PrevPrevPrevWord ;
    if(PrevWord.contains('.'))
        hasdot = true;
    qDebug()<<text_interval;

    QVector<QString> keys;
    QVector<QString> token_keys;
    QMap<QString,int> token_key_values;

    QString tokenkey = PrevPrevPrevWord.toLower() + " " + PrevPrevWord.toLower() + " " + PrevWord.toLower();
    tokenkey = tokenkey.trimmed();


    bool in_qutes = false;
    bool in_doublequtes = false;
    bool inComment = false;
    tokenkey = "";

    // grabbing data from tokenProcessor of requered depth
    for (int a = PrevWords.size() -1; a >= 0 ;a--)
    {
        //if(onWhiteSpace && a == 0)
        //    continue;


        //if(PrevWords[a].contains("--"))
        //    inComment=true;
        //if(inComment && PrevWords[a].contains("\n"))
        //{
        //    inComment = false;
        //    continue;
        //}
        //if(inComment)
        //    continue;

        int quotes = PrevWords[a].count('\'');
        if(quotes % 2 == 1)
        {
            in_qutes = !in_qutes;
            continue;
        }
        if(in_qutes)
            continue;


        int double_quotes = PrevWords[a].count('"');
        if(quotes % 2 == 1)
        {
            in_doublequtes = !in_doublequtes;
            continue;
        }
        if(in_doublequtes)
            continue;

        if(PrevWords[a].trimmed().size() < 1 || PrevWords[a].trimmed() ==" ")
            continue;
        QString str =  PrevWords[a].replace('\n',' ').replace('\t',' ').replace('\u0000',' ').toLower().trimmed();
        if((!isNumber(str) || (!isWord(str) && !isNumber(str))) && !str.contains('{')&& !str.contains(',') && !str.contains('}'))
        {
            tokenkey = str + " " + tokenkey;

            tokenkey = tokenkey.trimmed();
            qDebug() << "tk is: " << tokenkey << " } word" << str;
            for(auto s : tokenProcessor.ds.data[tokenkey.toStdString()])
            {
                keys.push_back(s.first.c_str());
                token_keys.push_back(s.first.c_str());
                token_key_values[s.first.c_str()] += (tokenProcessor.ds.GetPropertyAsFloat(tokenkey.toStdString(),s.first));
                qDebug() << s.first << tokenProcessor.ds.GetPropertyAsFloat(tokenkey.toStdString(),s.first);
            }
        }
    }


    // biases of probabilities of given key beeng next
    QVector<QString> morelikelykeys;
    QVector<QString> likelykeys;
    QVector<QString> lesslikelykeys;

    if(hasdot)
    { // its most likely to be a column of random table
        if(highlighter->TableColumnMap.contains(PrevPrevWord))
            for(auto ke : highlighter->TableColumnMap[PrevPrevWord].keys())
            {
                keys.push_back(ke);
                morelikelykeys.push_back(ke);
            }
        if(highlighter->TableColumnAliasMap.contains(PrevPrevWord))
            for(auto ke : highlighter->TableColumnMap[highlighter->TableColumnAliasMap[PrevPrevWord]].keys())
            {
                keys.push_back(ke);
                morelikelykeys.push_back(ke);
            }
        if(highlighter->tmpTableColumnMap.contains(PrevPrevWord))
            for(auto ke : highlighter->tmpTableColumnMap[PrevPrevWord].keys())
            {
                if(highlighter->tmpTableColumnMap[PrevPrevWord][ke])
                {
                    keys.push_back(ke);
                    morelikelykeys.push_back(ke);
                }
            }
    }
    else
    {
        keys.append(subCommandPatterns);
        if(highlighter->TableColumnMap.contains(PrevWord))
            for(auto ke : highlighter->TableColumnMap[PrevWord].keys())
            {
                keys.push_back(ke);
                lesslikelykeys.push_back(ke);
            }
        if(highlighter->TableColumnAliasMap.contains(PrevWord))
            for(auto ke : highlighter->TableColumnMap[highlighter->TableColumnAliasMap[PrevWord]].keys())
            {
                keys.push_back(ke);
                lesslikelykeys.push_back(ke);
            }
        for(auto ke : keywordPatterns)
        {
            keys.push_back(ke);
            morelikelykeys.push_back(ke);
        }
        for(auto ke : highlighter->TableColumnMap.keys())
        {
            keys.push_back(ke);
            morelikelykeys.push_back(ke);
        }
        for(auto ke : highlighter->tmpTableColumnMap.keys())
        {
            keys.push_back(ke);
            morelikelykeys.push_back(ke);
        }
        for(auto ke : highlighter->TableColumnAliasMap.keys())
        {
            if(highlighter->TableColumnAliasMap.contains(ke))
            {
                keys.push_back(ke);
                morelikelykeys.push_back(ke);
            }
        }
        //tmpTableColumnMap
    }


    // Levenshtein Distance-like method of finding relevant words on all gathered keys
    long int min_diff = 1000000000;
    QString mindiffWord = "";

    int wrong_cost = 10;
    int move_cost = 3;
    int add_cost = 1;
    int shorten_cost = 8;

    // select a word with minimal distance to current word
    for(auto k : keys)
    {
        long int diff = 0;
        int last_i = 0;
        float modifier = 100;

        if(token_keys.contains(k,Qt::CaseInsensitive)) // if its a processed token, make it more likely to show it
        {
            modifier -= (token_key_values[k] * 0.006f);
            //diff -=token_key_values[k];
        }
        if(likelykeys.contains(k,Qt::CaseInsensitive))
        {
            modifier -= (1.55f);
            //diff -=token_key_values[k];
        }
        if(morelikelykeys.contains(k,Qt::CaseInsensitive))
        {
            modifier -= (3.0f);
            //diff -=token_key_values[k];
        }
        if(lesslikelykeys.contains(k,Qt::CaseInsensitive))
        {
            modifier += (4.25f);
            //diff -=token_key_values[k];
        }

        for(int i=0;i<word.size();i++)
        {
            int mdiff = wrong_cost;
            for(int a = 0 ; a<k.size();a++)
            {
                if(k[a].toLower()==word[i].toLower() && move_cost * abs(i-a)<mdiff)
                    mdiff = move_cost * abs(i-a);
            }
            diff+=mdiff;
            last_i = i;
        }
        if(k.size()<word.size())
            diff+=abs(k.size()-word.size()) * shorten_cost;
        else
            diff+=abs(k.size()-word.size()) * add_cost;
        diff *= modifier;
        if(diff<min_diff)
        {
            min_diff = diff;
            mindiffWord = k;
        }

    }

    // save the word
    lastSuggestedWord = mindiffWord;

    // detect if this word is table column
    lastwordisTableColumn = false;
    if(highlighter->TableColumnMap.contains(PrevWord))
        for(auto ke : highlighter->TableColumnMap[PrevWord].keys())
        {
            if(lastSuggestedWord == ke)
            {
                lastwordisTableColumn = true;
                break;
            }
        }
    if(!lastwordisTableColumn)
        if(highlighter->TableColumnAliasMap.contains(PrevWord))
            for(auto ke : highlighter->TableColumnMap[highlighter->TableColumnAliasMap[PrevWord]].keys())
            {
                if(lastSuggestedWord == ke)
                {
                    lastwordisTableColumn = true;
                    break;
                }
            }

    if(!lastwordisTableColumn)
        for(auto ke : highlighter->TableColumnMap.keys())
        {
            if(lastSuggestedWord == ke)
            {
                lastwordisTableColumn = true;
                break;
            }
        }
    if(!lastwordisTableColumn)
        for(auto ke : highlighter->TableColumnAliasMap.keys())
        {
            if(highlighter->TableColumnAliasMap.contains(ke))
                if(lastSuggestedWord == ke)
                {
                    lastwordisTableColumn = true;
                    break;
                }
        }
    for(auto ke : highlighter->tmpTableColumnMap.keys())
    {
        if(lastSuggestedWord == ke)
        {
            lastwordisTableColumn = true;
            break;
        }
    }

    if(hasdot)
    {
        if(highlighter->TableColumnMap.contains(PrevPrevWord))
            for(auto ke : highlighter->TableColumnMap[PrevPrevWord].keys())
            {
                if(lastSuggestedWord == ke)
                {
                    lastwordisTableColumn = true;
                    break;
                }
            }
        if(highlighter->TableColumnAliasMap.contains(PrevPrevWord))
            for(auto ke : highlighter->TableColumnMap[highlighter->TableColumnAliasMap[PrevPrevWord]].keys())
            {
                if(lastSuggestedWord == ke)
                {
                    lastwordisTableColumn = true;
                    break;
                }
            }
        if(highlighter->tmpTableColumnMap.contains(PrevPrevWord))
            for(auto ke : highlighter->tmpTableColumnMap[PrevPrevWord].keys())
            {
                if(highlighter->tmpTableColumnMap[PrevPrevWord][ke])
                {
                    if(lastSuggestedWord == ke)
                    {
                        lastwordisTableColumn = true;
                        break;
                    }
                }
            }
    }

    emit s_suggestedName();

}
// depricated
void CodeEditor::fillName()
{
    FillsuggestName();
}

void CodeEditor::FillsuggestName()
{
    QTextCursor cursor = textCursor();
    QString lasttext = lastSuggestedWord;

    //decide if its a real word, or a tab symbol needs to be placed
    if(cursor.atBlockStart() || (lasttext.size()<=1 && (lasttext.size() > 0 && lasttext[0]!='.'&& lasttext[0]!='='&& lasttext[0]!='*'&& lasttext[0]!='\''&& lasttext[0]!='(' && lasttext[0]!=')' )))
    {
        cursor.insertText("\t");
        return;
    }

    bool lasttexttablecolumn = lastwordisTableColumn;
    QString word = "";
    QString text = toPlainText();

    QTextCursor tx =textCursor();

    tx.movePosition(QTextCursor::StartOfBlock);
    int block_start = tx.position();

    // get prev word end
    int curs = textCursor().position();
    if(curs>=1)
        curs-=1;
    int curscop = curs;
    int start = curs;
    int end = curs;
    bool hitspace = false;

    bool postgreSQL = highlighter->PostgresStyle;
    while(text[curscop].isLetter() || text[curscop] == '_'||(text[curscop] == '"' && postgreSQL)|| text[curscop] == '\t'||text[curscop] == '$'||text[curscop] == ' ' )
    {
        if(hitspace && (text[curscop] != ' '))
        {
            curscop++;
            curscop++;
            start = curscop;
            break;
        }
        start = curscop;
        if(text[curscop] == ' ')
        {
            hitspace = true;
        }
        curscop-=1;
    }
    while(text[curs].isLetter() ||text[curs] == '_'|| (text[curs] == '"' && postgreSQL) || text[curs] == '\t'||text[curs] == '$')
    {
        end = curs;
        curs+=1;
    }
    if(text[start]=='.')
        start++;
    for(int i=start;i<=end;i++)
    {
        word+=text[i];

    }

    qDebug() << word;
    bool CursedStarSymbolDetected = false;
    if(start-1 >=0 && ((isSpecialSymbol(text[start-1]) && text[start-1] != '"' && text[start-1] != '\'' && text[start-1] != '.') || text[start-1].isDigit()))
        CursedStarSymbolDetected = true;

    qDebug()<< "Pasted: "<<lastSuggestedWord;
    cursor.beginEditBlock();

    if(block_start  > start)
        cursor.setPosition(block_start);
    else
    {
        if(lasttext.back() == '.')
            cursor.setPosition(start-1, QTextCursor::MoveAnchor);
        else
            cursor.setPosition(start, QTextCursor::MoveAnchor);
    }

    cursor.setPosition(end+1, QTextCursor::KeepAnchor);

    if(CursedStarSymbolDetected && lasttext.back() != '"' && lasttext.back() != '\'' && lasttext.back() != '.')
           cursor.insertText(" ");
    if(postgreSQL && lasttexttablecolumn)
        cursor.insertText("\"");

    cursor.insertText(lasttext);

    if(postgreSQL && lasttexttablecolumn)
        cursor.insertText("\"");
    cursor.insertText(" ");

    cursor.endEditBlock();

}

void CodeEditor::CommentSelected()
{// get start, get end, paste '--' at every block start

    QTextCursor cursor = textCursor();
    int s_start = cursor.selectionStart();
    int s_end = cursor.selectionEnd();

    cursor.setPosition(s_start);
    int b_start = cursor.block().blockNumber();
    cursor.setPosition(s_end);
    int b_end = cursor.block().blockNumber();

    bool uncoment = false;
    cursor.setPosition(s_start);

    if(cursor.block().text().startsWith("--"))
        uncoment = true;
    else
        uncoment = false;


    QTextBlock tb = cursor.block();

    cursor.beginEditBlock();
    while(tb.blockNumber() <= blockCount() && tb.isValid()  && tb.blockNumber() <= b_end)
    {
        cursor.setPosition(tb.position());
        if(uncoment && tb.text().startsWith("--"))
        {
            cursor.deleteChar();
            cursor.deleteChar();
        }
        else if (!tb.text().startsWith("--"))
        {
            cursor.insertText("--");
        }

        tb = tb.next();
    }
    cursor.endEditBlock();

}

QStringList CodeEditor::GetTokensUnderCursor()
{
    QStringList out_strl;
    QString word = "";
    QString PrevWord= "";
    QString PrevPrevWord= "";
    QString PrevPrevPrevWord= "";
    QStringList PrevWords;
    int depth = 6; // 3 prevwords
    QString text = toPlainText();
    int curs = textCursor().position();
    if(curs>=1)
        curs-=1;
    int curscop = curs;
    int start = curs;
    int end = curs;



    while(text[curscop].isLetter() ||text[curscop] == '_'||text[curscop] == '\"'||text[curscop] == '$' )
    {
        start = curscop;
        curscop-=1;
        if(curscop<0)
            break;
    }
    while(text[curs].isLetter() ||text[curs] == '_'|| text[curs] == '\"' ||text[curs] == '$')
    {
        end = curs;
        curs+=1;
        if(curs>text.size())
            break;
    }
    for(int i=start;i<=end;i++)
    {
        if(text[i]!='"')
            word+=text[i];
    }

    int curs2 = start - 1;
    if(curs2>=1)
        curs2-=1;
    int curscop2 = curs2;
    int start2 = curs2;
    int end2 = curs2;
    for(int i =0;i < depth; i++)
    {
        curs2 = start2 - 1;
        if(curs2>=1)
            curs2-=1;
        curscop2 = curs2;
        start2 = curs2;
        end2 = curs2;
        while(text[curscop2].isLetter() ||text[curscop2] == '_'||text[curscop2] == '\"'||text[curscop2] == '$' )
        {
            start2 = curscop2;
            curscop2-=1;
            if(curscop2<0)
                break;
        }
        while(text[curs2].isLetter() ||text[curs2] == '_'||text[curs2] == '\"'||text[curs2] == '$')
        {
            end2 = curs2;
            curs2+=1;
            if(curs>text.size())
                break;
        }
        QString wrd = "";
        for(int i=start2;i<=end2;i++)
        {
            if(text[i]!='"')
                wrd+=text[i];
        }

        //PrevWords.push_back(wrd.replace('\n',' ').replace('\u0000',' ').toLower().trimmed());
    }



    word = word;
    PrevWord = "";
    PrevPrevWord = "";
    PrevPrevPrevWord = "";

    QString text_interval = "";
    for(int i=start2; i <= end && i < text.size();i++)
    {
        text_interval += text[i];
    }

    PrevWords.clear();
    QStringList text_interval_strl = processBlockToTokens(text_interval);

    if(textCursor().position()-1 >=0 && (text[textCursor().position()-1] == ' ' || text[textCursor().position()-1] == '\t'|| text[textCursor().position()-1] == '\n'))
    {
        if(text_interval_strl.size() >= 3)
            PrevPrevPrevWord = text_interval_strl[text_interval_strl.size()-3].toLower();
        if(text_interval_strl.size() >= 2)
            PrevPrevWord = text_interval_strl[text_interval_strl.size()-2].toLower();
        if(text_interval_strl.size() >= 1)
            PrevWord = text_interval_strl[text_interval_strl.size()-1].toLower();

        for(int i=0; i < text_interval_strl.size();i++)
        {
            PrevWord.push_back( text_interval_strl[i].replace('\n',' ').replace('\t',' ').replace('\u0000',' ').toLower().trimmed());
        }
        PrevWord = PrevWord[2];
        PrevPrevWord = PrevWord[1];
        PrevPrevPrevWord = PrevWord[0];
        word = "";
        qDebug() << "On whitespace";
    }
    else
    {

        if(text_interval_strl.size() >= 4)
            PrevPrevPrevWord= text_interval_strl[text_interval_strl.size()-4].toLower();
        if(text_interval_strl.size() >= 3)
            PrevPrevWord = text_interval_strl[text_interval_strl.size()-3].toLower();
        if(text_interval_strl.size() >= 2)
            PrevWord = text_interval_strl[text_interval_strl.size()-2].toLower();
        if(text_interval_strl.size() >= 1)
            word = text_interval_strl[text_interval_strl.size()-1].toLower();

        for(int i=0; i < text_interval_strl.size();i++)
        {
            PrevWord.push_back( text_interval_strl[i].replace('\n',' ').replace('\t',' ').replace('\u0000',' ').toLower().trimmed());
        }

        PrevWord = PrevWord[3];
        PrevPrevWord = PrevWord[2];
        PrevPrevPrevWord = PrevWord[1];
        word = PrevWord[0];
    }

    word = word.replace('\u0000',' ').trimmed().toLower();

    out_strl.push_back(PrevPrevPrevWord);
    out_strl.push_back(PrevPrevWord);
    out_strl.push_back(PrevWord);
    out_strl.push_back(word);
    return out_strl;
}
