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
    lineNumberArea = new LineNumberArea(this);

    updateLineNumberAreaWidth(0);
    highlightCurrentLine();

    QFont font;
    font.setFamily("Courier");
    font.setFixedPitch(true);
    font.setPointSize(10);
    setFont(font);

    highlighter = new Highlighter(document());

    setTabStopDistance(QFontMetricsF(this->font()).horizontalAdvance(' ') * 4);

    connect(this,&CodeEditor::cursorPositionChanged,this,&CodeEditor::suggestName, Qt::QueuedConnection );

    connect(this, &CodeEditor::blockCountChanged, this, &CodeEditor::updateLineNumberAreaWidth);
    connect(this, &CodeEditor::updateRequest, this, &CodeEditor::updateLineNumberArea);

    connect(this, &CodeEditor::cursorPositionChanged, this, &CodeEditor::highlightCurrentLine);


    //connect( cd, SIGNAL(s_suggestedName()), this, SLOT(updatesuggestion()), Qt::QueuedConnection );

    connect( this, SIGNAL(tabDetected()), this, SLOT(fillName()), Qt::QueuedConnection );

    if(userDS.Load("userdata.txt"))
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
    //highlighter->OnBlockCountChanged(newBlockCount);
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

    if(abs(textCursor().selectionEnd() - textCursor().selectionStart()) >= 1)// selected something
    {
        QTextEdit::ExtraSelection wordSelection;

        int bracketStart = cursor_position;
        if(cursor_position-1 >0 && text[cursor_position-1] == ')')
            bracketStart = cursor_position-1;


        QString selectedString = textCursor().selectedText();
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
                selection.format.setBackground(QColor(Qt::white).lighter(20));
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


void CodeEditor::replace(int _from, int _to, QString _what, QString _with)
{// better to reimplement to work from cursors, not just text replacing
    if(_from > _to)
        return;
    if(_what.size() < 1 || _with.size()<1)
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
        if(highlighter->tmpTableColumnMap.contains(PrevWord))
            for(auto ke : highlighter->tmpTableColumnMap[PrevWord].keys())
            {
                if(highlighter->tmpTableColumnMap[PrevWord][ke])
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
        for(auto ke : highlighter->tmpTableColumnMap.keys())
        {
            keys.push_back(ke);
        }
        for(auto ke : highlighter->TableColumnAliasMap.keys())
        {
            if(highlighter->TableColumnAliasMap.contains(ke))
                keys.push_back(ke);
        }
        //tmpTableColumnMap
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
    if(lasttext.size()<=1)
    {
        cursor.insertText("\t");
        return;
    }
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
    bool CursedStarSymbolDetected = false;
    if(start-1 >=0 && (text[start-1] =='*' || text[start-1] =='>'|| text[start-1] =='<'|| text[start-1] =='='|| text[start-1] =='+'|| text[start-1] =='-'))
        CursedStarSymbolDetected = true;
    qDebug()<< "Pasted: "<<lastSuggestedWord;
    cursor.beginEditBlock();
    cursor.setPosition(start, QTextCursor::MoveAnchor);
    cursor.setPosition(end+1, QTextCursor::KeepAnchor);
    if(CursedStarSymbolDetected)
        cursor.insertText(" ");
    if(postgreSQL && lasttexttablecolumn)
        cursor.insertText("\"");
    cursor.insertText(lasttext);
    if(postgreSQL && lasttexttablecolumn)
        cursor.insertText("\"");
    cursor.endEditBlock();

}

void CodeEditor::CommentSelected()
{
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



