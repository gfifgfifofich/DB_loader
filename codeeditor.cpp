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

inline DataStorage userDS;

CodeEditor::CodeEditor(QWidget *parent) : QPlainTextEdit(parent)
{
    lineNumberArea = new LineNumberArea(this);

    connect(this, &CodeEditor::blockCountChanged, this, &CodeEditor::updateLineNumberAreaWidth);
    connect(this, &CodeEditor::updateRequest, this, &CodeEditor::updateLineNumberArea);
    connect(this, &CodeEditor::cursorPositionChanged, this, &CodeEditor::highlightCurrentLine);

    updateLineNumberAreaWidth(0);
    highlightCurrentLine();

    QFont font;
    font.setFamily("Courier");
    font.setFixedPitch(true);
    font.setPointSize(10);
    setFont(font);

    highlighter = new Highlighter(document());


    connect(this,&CodeEditor::cursorPositionChanged,this,&CodeEditor::suggestName, Qt::QueuedConnection );


    connect( this, SIGNAL(tabDetected()), this, SLOT(fillName()), Qt::QueuedConnection );
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
void CodeEditor::updateLineNumberAreaWidth(int /* newBlockCount */)
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
void CodeEditor::highlightCurrentLine()
{
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

    setExtraSelections(extraSelections);
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


void CodeEditor::suggestName()
{

    QString word = "";
    QString PrevWord= "";
    QString text = toPlainText();
    int curs = textCursor().position();
    if(curs>=1)
        curs-=1;
    int curscop = curs;
    int start = curs;
    int end = curs;

    if( (text.size() > curs && curs > 0 && text[curs] == '\t') && (prevTabPos != curs || prevTabTextSize != text.size()))
    {
        if(lastSuggestedWord.size()<2)
            lastSuggestedWord = "    ";
        prevTabPos = curs;
        prevTabTextSize = text.size();
        FillsuggestName();
        return;
    }


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
    for(int i=start2;i<=end2;i++)
    {
        if(text[i]!='"')
            PrevWord+=text[i];
    }
    word = word;
    PrevWord = PrevWord;
    bool hasdot = false;
    for(int i=start2; i < end;i++)
    {
        if(text[i]=='.')
        {
            hasdot=true;
            break;
        }
    }



    QVector<QString> keys;
    if(hasdot)
    {
        if(highlighter->TableColumnMap.contains(PrevWord))
            for(auto ke : highlighter->TableColumnMap[PrevWord].keys())
            {
                keys.push_back(ke);
            }
        if(highlighter->TableColumnAliasMap.contains(PrevWord))
            for(auto ke : highlighter->TableColumnMap[highlighter->TableColumnAliasMap[PrevWord]].keys())
            {
                keys.push_back(ke);
            }
    }
    else
    {
        if(highlighter->TableColumnMap.contains(PrevWord))
            for(auto ke : highlighter->TableColumnMap[PrevWord].keys())
            {
                keys.push_back(ke);
            }
        if(highlighter->TableColumnAliasMap.contains(PrevWord))
            for(auto ke : highlighter->TableColumnMap[highlighter->TableColumnAliasMap[PrevWord]].keys())
            {
                keys.push_back(ke);
            }
        for(auto ke : keywordPatterns)
        {
            keys.push_back(ke);
        }
        for(auto ke : highlighter->TableColumnMap.keys())
        {
            keys.push_back(ke);
        }
        for(auto ke : highlighter->TableColumnAliasMap.keys())
        {
            if(highlighter->TableColumnAliasMap.contains(ke))
                keys.push_back(ke);
        }
    }


    // Levenshtein Distance-like method of finding relevant words

    long int min_diff = 1000000000;
    QString mindiffWord = "";

    int wrong_cost = 8;
    int move_cost = 1;
    int add_cost = 1;
    int shorten_cost = 4;

    for(auto k : keys)
    {
        long int diff = 0;
        int last_i = 0;
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

        if(diff<min_diff)
        {
            min_diff = diff;
            mindiffWord = k;
        }

    }
    lastSuggestedWord = mindiffWord;

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


    emit s_suggestedName();

}

void CodeEditor::fillName()
{
    FillsuggestName();
}


void CodeEditor::FillsuggestName()
{
    QTextCursor cursor = textCursor();
    QString lasttext = lastSuggestedWord;
    bool lasttexttablecolumn = lastwordisTableColumn;
    QString word = "";
    QString text = toPlainText();
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

    qDebug()<< "Pasted: "<<lastSuggestedWord;
    cursor.setPosition(start, QTextCursor::MoveAnchor);
    cursor.setPosition(end+1, QTextCursor::KeepAnchor);
    if(postgreSQL && lasttexttablecolumn)
        cursor.insertText("\"");
    cursor.insertText(lasttext);
    if(postgreSQL && lasttexttablecolumn)
        cursor.insertText("\"");

}




