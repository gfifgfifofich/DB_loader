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
#include <QProxyStyle>
#include <QScrollBar>
#include <QtGui>
#include "sqlSubfunctions.h"
#include "tokenprocessor.h"
#include <math.h>
#include <qstyleoption.h>




class VProxyStyle : public QProxyStyle
{
public:
    int scrollBarEdgeOffset = 0;

    VProxyStyle(QStyle *style = nullptr) : QProxyStyle(style) { }

    QRect subControlRect(QStyle::ComplexControl cc, const QStyleOptionComplex* opt, QStyle::SubControl sc, const QWidget *widget = nullptr) const override
    {
        if (cc == CC_ScrollBar || sc == QStyle::SC_ScrollBarGroove)
        {
            QRect rect = QProxyStyle::subControlRect(cc, opt, sc, widget);
            // Exclude the top and bottom arrows area.

            rect.setTop(rect.top() - scrollBarEdgeOffset);
            rect.setBottom(rect.bottom() + scrollBarEdgeOffset);
            return rect;
        }

        return QProxyStyle::subControlRect(cc, opt, sc, widget);
    }
};

class VScrollBar : public QScrollBar
{

public:
    CodeEditor* cd = nullptr;
    VScrollBar(Qt::Orientation orientation, QWidget *parent) : QScrollBar(orientation, parent)
    {
        VProxyStyle *proxyStyle = new VProxyStyle(style());
        setStyle(proxyStyle);

        QStyleOptionSlider scrollBarStyleOption;
        scrollBarStyleOption.initFrom(this);

        proxyStyle->scrollBarEdgeOffset = this->style()->subControlRect(QStyle::CC_ScrollBar,
                                                                        &scrollBarStyleOption,
                                                                        QStyle::SC_ScrollBarAddLine,
                                                                        this).width();
    }



    void paintEvent(QPaintEvent *event) override
    {

        QPainter painter(this);
        QRect sliderRect = sliderHandleRect();

        sliderRect.setX(sliderRect.width()*0.9f);
        sliderRect.setWidth(sliderRect.width());

        QRect arearect(0,0,width(),height());



        if(cd != nullptr && cd->updateZoomOutViewOnScroll)
        {
            QPainterPath path;
            cd->updateZoomOutViewOnScroll = false;
            cd->getScreenshot(cd->zoomOutView);
            painter.drawPixmap(arearect, cd->zoomOutView, cd->zoomOutView.rect());
            path.addRect(arearect);
            painter.setPen(QColor::fromRgbF(0.7f,0.7f,0.7f,0.1f));
            painter.fillPath(path, QColor::fromRgbF(0.7f,0.7f,0.7f,0.1f));

            painter.drawPath(path);
            cd->updateZoomOutViewOnScroll=true;
        }


        QPainterPath path;
        path.addRect(sliderRect);
        painter.setPen(QColor::fromRgbF(0.7f,0.7f,0.7f,0.3f));
        painter.fillPath(path, QColor::fromRgbF(0.7f,0.7f,0.7f,0.3f));
        painter.drawPath(path);


    }

    QRect sliderHandleRect() const
    {
        QStyleOptionSlider opt;
        initStyleOption(&opt);
        return style()->subControlRect(QStyle::CC_ScrollBar, &opt, QStyle::SC_ScrollBarSlider, this);
    }
};

void CodeEditor::getScreenshot(QPixmap &map)
{
    QSize sizebuff = size();
    map = QPixmap(size().width()*0.75f,size().height()* 1.0f);


    QPainter painter(&map);
    painter.scale(0.75f,1.0f);

    float offset = 0;
    int end_at = 200;
    float step = float(size().height()) /end_at;
    if(step <= 0)
        step = 1;


    QTextBlock fblock = firstVisibleBlock();
    int top = fblock.blockNumber();
    int bottom = top;
    while (fblock.isValid() && fblock.isVisible()) {
        bottom = fblock.blockNumber();
        fblock = fblock.next();

    }



    QTextBlock block;
    if(this->firstVisibleBlock().blockNumber()-end_at*0.5f >=0)
        block = document()->findBlockByNumber( this->firstVisibleBlock().blockNumber()-end_at*0.5f);
    else
        block = document()->findBlockByNumber( 0);


    if(document()->blockCount()-end_at<=0)
        end_at =document()->blockCount();

    if(block.blockNumber() + end_at > document()->blockCount())
    {
        block = document()->findBlockByNumber( document()->blockCount()-end_at);
    }



    int i = block.blockNumber();
    float topvisibleoffset = offset;
    float bottomvisibleoffset = offset;
    while (end_at >0 && i < highlighter->lineInterval.size())
    {
        end_at--;
        int xoffset = 0;


        // if(i>=top)
        //     topvisibleoffset = offset;

        // if(i<=bottom)
        //     bottomvisibleoffset = offset;

        for(int a = 0; a < highlighter->lineInterval[i].size();a++)
        {
            //rects[highlighter->lineIntervalColor[i][a]].emplace_back(xoffset*10,offset*1,highlighter->lineInterval[i][a]*10,step*1);
            painter.setBrush(highlighter->lineIntervalColor[i][a]);
            painter.drawRect(highlighter->lineInterval[i][a].first*10,offset*1,highlighter->lineInterval[i][a].second*10,step*1);
        }

        if(i>=top && i<=bottom)
        {
            painter.setBrush(QColor(255,255,255,20));
            painter.drawRect(0,offset*1,1500,step*1);
        }
        offset += step;
        i++;
    }
}


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
    if(e->key() == 16777249)
    {
        ctrl_pressed = true;
    }
    if(e->key() == 16777248)
    {
        shift_pressed = true;
    }

    if(e->key() == 16777217 && !shift_pressed)
    {
        FillsuggestName();
        return;
    }
    if(e->key() == 16777220) // enter
    {//
        QTextCursor c = this->textCursor();
        QString str =  c.block().text();
        QString prefix = "";
        int pos =0;
        while(pos<str.size() && (str[pos]==' ' || str[pos]=='\t'))
        {
            prefix+=str[pos];
            pos++;
        }
        c.beginEditBlock();
        QPlainTextEdit::keyPressEvent(e);
        c.insertText(prefix);
        c.endEditBlock();
        return;
    }
    QPlainTextEdit::keyPressEvent(e);
}

void CodeEditor::keyReleaseEvent(QKeyEvent *e)
{
    //16777217 tab
    //16777220 return
    //16777248 SHIFT
    //16777249 ctrl
    //qDebug()<< e->key();
    if(e->key() == 16777249)
    {
        ctrl_pressed = false;
    }
    if(e->key() == 16777248)
    {
        shift_pressed = false;
    }
    QPlainTextEdit::keyReleaseEvent(e);
}
CodeEditor::CodeEditor(QWidget *parent) : QPlainTextEdit(parent)
{
    lineNumberArea = new LineNumberArea(this);
    codePreview = new LineNumberArea(this);
    ((LineNumberArea*) codePreview)->id = 1;
    qDebug() << "reached Highlighter";
    highlighter = new Highlighter(document());

    qDebug() << "news done";
    updateLineNumberAreaWidth(0);



    QFont font;
    font.setFamily("Courier NEW");
    font.setFixedPitch(true);
    font.setPointSize(10);
    setFont(font);



    connect(this,&CodeEditor::cursorPositionChanged,this,&CodeEditor::suggestName, Qt::QueuedConnection );
    connect(this, &CodeEditor::blockCountChanged, this, &CodeEditor::updateLineNumberAreaWidth);
    connect(this, &CodeEditor::updateRequest, this, &CodeEditor::updateLineNumberArea);
    connect(this, &CodeEditor::cursorPositionChanged, this, &CodeEditor::highlightCurrentLine);


    // autocompleation on tab press
    connect( this, SIGNAL(tabDetected()), this, SLOT(fillName()), Qt::QueuedConnection );
    setTabStopDistance(QFontMetricsF(this->font()).horizontalAdvance(' ') * 4);



    updateMisc();



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
    if(b_codePreview)
        setViewportMargins(lineNumberAreaWidth(), 0, 190, 0);
    else
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

    if(b_codePreview)
    {
        if (dy)
            codePreview->scroll(viewport()->rect().left() - 200, dy);
        else
            codePreview->update(viewport()->rect().left() - 200, rect.y(), 200, rect.height());
    }


}
void CodeEditor::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);

    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
    if(b_codePreview)
        codePreview->setGeometry(QRect(cr.right()-200, cr.top(), 200, cr.height()));
}
void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{

    QPainter painter(lineNumberArea);

    if(QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Light)
        painter.fillRect(event->rect(), Qt::white);
    else
        painter.fillRect(event->rect(), Qt::black);

    QTextBlock block = firstVisibleBlock();

    int blockNumber = block.blockNumber();

    int topblock = block.blockNumber();
    int bottomblock = topblock;

    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());
    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);

            if(QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Light)
                painter.setPen(Qt::black);
            else
                painter.setPen(Qt::white);

            painter.drawText(0, top, lineNumberArea->width(), fontMetrics().height(),
                             Qt::AlignRight, number);
        }

        block = block.next();
        bottomblock = block.blockNumber();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++blockNumber;
    }

}

void CodeEditor::drawPreview(QPaintEvent *event)
{



    QTextBlock block = firstVisibleBlock();

    int blockNumber = block.blockNumber();

    int topblock = block.blockNumber();
    int bottomblock = topblock;

    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());
    while (block.isValid() && top <= event->rect().bottom()) {

        block = block.next();
        bottomblock = block.blockNumber();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++blockNumber;
    }

    if(bottomblock - topblock <= 0)
    {
        bottomblock = this->blockCount();
        //topblock = bottomblock - 20;
    }

    int end_at_mult = bottomblock - topblock;

    int snapcount = userDS.GetPropertyAsInt("UserTheme", "CodePreviewSnapCount");
    if(snapcount <=0)
        snapcount = 1;
    end_at_mult = (end_at_mult / snapcount) * snapcount;

    int end_at = userDS.GetPropertyAsInt("UserTheme", "CodePreviewLineCount");

    if(end_at_mult <= end_at)
        end_at_mult = end_at;
    end_at = end_at_mult * end_at;

    if(b_codePreview && (bottomblock - topblock < 0.0f || end_at_mult >= 11 || end_at >= this->blockCount()))
    {



        QPixmap px;

        QSize sizebuff = size();

        // random noize appears when attempting to render small pixmap, so scale x depending on size y
        int x_mult = 1;
        if(end_at_mult >40)
        {
            px = QPixmap(size().width() * 0.3f,size().height());
            x_mult = 3;
        }
        else if(end_at_mult >20)
        {
            px = QPixmap(size().width() * 0.5f,size().height());
            x_mult = 4;
        }
        else
        {
            px = QPixmap(size().width(),size().height());
            x_mult = 8;
        }
        x_mult *=0.8f;
        QPainter CPpainter(&px);

        if(QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Light)
            CPpainter.fillRect(px.rect(), Qt::white);
        else
            CPpainter.fillRect(px.rect(), Qt::black);

        if(userDS.GetProperty("UserTheme", "CodePreviewAntialiasing") == "true")
        {
            CPpainter.setRenderHint(CPpainter.Antialiasing);
        }
        float offset = 0;

        float step = float(size().height()) /end_at;
        if(step <= 0)
            step = 1;



        QTextBlock block;
        if(this->firstVisibleBlock().blockNumber()-end_at*0.5f >=0)
            block = document()->findBlockByNumber( this->firstVisibleBlock().blockNumber()-end_at*0.5f);
        else
            block = document()->findBlockByNumber( 0);


        if(document()->blockCount()-end_at<=0)
            end_at =document()->blockCount();

        if(block.blockNumber() + end_at > document()->blockCount())
        {
            block = document()->findBlockByNumber( document()->blockCount()-end_at);
        }



        std::vector<QColor> cols;
        std::vector<std::vector<QRect>> rects;

        int i = block.blockNumber();
        float topvisibleoffset = offset;
        float bottomvisibleoffset = offset;
        while (end_at >0 && i < highlighter->lineInterval.size())
        {
            end_at--;
            int xoffset = 0;


            if(i<=topblock)
                topvisibleoffset = offset;

            if(i<=bottomblock)
                bottomvisibleoffset = offset;

            for(int a = 0; a < highlighter->lineInterval[i].size();a++)
            {

                int colID = -1;
                for(int cl =0; cl< cols.size();cl++)
                    if(cols[cl] == highlighter->lineIntervalColor[i][a])
                    {
                        colID = cl;
                        break;
                    }
                if(colID==-1)
                {
                    cols.push_back(highlighter->lineIntervalColor[i][a]);
                    colID = cols.size()-1;
                }
                while (colID >= rects.size())
                    rects.emplace_back();

                rects[colID].emplace_back(highlighter->lineInterval[i][a].first*x_mult,offset*1,highlighter->lineInterval[i][a].second*x_mult,step*1);
            }
            offset += step;
            i++;
        }


        for(i =0; i< rects.size();i++)
        {
            if(highlighter->multiLineCommentFormat.foreground().color() != cols[i] && highlighter->quotationFormat.foreground().color() != cols[i])
            {
                CPpainter.setBrush(cols[i]);
                CPpainter.drawRects(rects[i].data(),rects[i].size());
            }
        }


        for(i =0; i< rects.size();i++)
        {
            if(highlighter->quotationFormat.foreground().color() == cols[i])
            {
                CPpainter.setBrush(cols[i]);
                CPpainter.drawRects(rects[i].data(),rects[i].size());
            }
        }


        for(i =0; i< rects.size();i++)
        {
            if(highlighter->multiLineCommentFormat.foreground().color() == cols[i])
            {
                CPpainter.setBrush(cols[i]);
                CPpainter.drawRects(rects[i].data(),rects[i].size());
            }
        }



        if(end_at <= this->blockCount())
        {
            CPpainter.setBrush(QColor(255,255,255,20));
            if(bottomvisibleoffset - topvisibleoffset > 0.0f)
                CPpainter.drawRect(0,topvisibleoffset,1800,bottomvisibleoffset - topvisibleoffset);
            else
                CPpainter.drawRect(0,topvisibleoffset,1800,500);
        }



        QPainter cpainter(codePreview);
        cpainter.fillRect(codePreview->rect(), Qt::black);
        cpainter.drawPixmap(codePreview->rect(),px);

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

        int commentcoumter = 0;
        bool inComment = false;

        for (int i = bracketStart; i < text.size(); i++)
        {
            if(!inComment)
            {
                if(text[i]=='-')
                    commentcoumter++;
                else
                    commentcoumter = 0;
                if(commentcoumter >= 2 )
                {
                    commentcoumter = 0;
                    inComment = true;
                }

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
            else
            {
                if(text[i]=='\n')
                    inComment =false;
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

        int commentcoumter = 0;
        bool inComment = false;
        int bracketamtlastline = 0;
        bool waitnextline = false;
        for (int i = bracketStart; i >= 0; i--)
        {
            if(text[i]=='-')
                commentcoumter++;
            else
                commentcoumter = 0;
            if(commentcoumter >= 2 )
            {
                commentcoumter = 0;
                bracketCount -= bracketamtlastline;
                bracketamtlastline = 0;
                waitnextline = false;
            }
            if(text[i]=='\n')
                bracketamtlastline = 0;
            if(waitnextline && bracketamtlastline == 0)
                break;

            if(text[i]=='(')
            {
                bracketCount ++;
                bracketamtlastline++;

                if(bracketCount >=0)
                {
                    bracketEnd=i;
                    waitnextline = true;
                }
            }

            if(text[i]==')')
            {
                bracketCount --;
                bracketamtlastline--;
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
    int depth = 30; // amount of prevwords to check
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
        if(curs2 <0)
        {
            curs2=0;
            break;
        }
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
    if(word.back() == '.')
        word="";

    //qDebug() << "word is " <<word << onWhiteSpace;
    // qDebug() << "PrevWord is " <<PrevWord ;
    // qDebug() << "PrevPrevWord is " <<PrevPrevWord ;
    // qDebug() << "PrevPrevPrevWord is " <<PrevPrevPrevWord ;

    if(PrevWord.contains('.'))
        hasdot = true;
    //qDebug()<<text_interval;

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
    if(onWhiteSpace ) // currently, due to really bad data from tokenprocessor, its data is disabled from autocompleate, in case user started typing something
        // skip current word, to not ruin map find. Ruins everyting else due to bad data from tokenprocessor

    for (int a = PrevWords.size() -1; a >= 0 ;a--)
    {


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
        if((!isNumber(str) || str == "." || (!isWord(str) && !isNumber(str))) && !str.contains('{')&& !str.contains(',') && !str.contains('}'))
        {
            tokenkey = str + " " + tokenkey;

            tokenkey = tokenkey.trimmed();
            //qDebug() << "tk is: " << tokenkey << " } word" << str;
            for(auto s : tokenProcessor.ds.data[tokenkey])
            {
                keys.push_back(s.first);
                token_keys.push_back(s.first);
                token_key_values[s.first] += (tokenProcessor.ds.GetPropertyAsFloat(tokenkey,s.first));
                //qDebug() << s.first << tokenProcessor.ds.GetPropertyAsFloat(tokenkey,s.first);
            }
        }
    }


    // biases of probabilities of given key beeng next
    QVector<QString> morelikelykeys;
    QVector<QString> likelykeys;
    QVector<QString> lesslikelykeys;


    int _tmp_key_size = keys.size();
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
        if(_tmp_key_size == keys.size())
        {// 0 results in these tables, so probably its a schema name or smth
            // add table names, cuz there is no point in keywords after dot
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
    QString text = toPlainText();
    QTextCursor cursor = textCursor();
    QString lasttext = lastSuggestedWord;

    if(cursor.hasSelection())
    {
        int s_start = cursor.selectionStart();
        int s_end = cursor.selectionEnd();

        cursor.setPosition(s_start);
        int b_start = cursor.block().blockNumber();
        cursor.setPosition(s_end);
        int b_end = cursor.block().blockNumber();

        cursor.setPosition(s_start);

        QTextBlock tb = cursor.block();


        cursor.beginEditBlock();

        if(!ctrl_pressed)
        {
            while(tb.blockNumber() <= blockCount() && tb.isValid()  && tb.blockNumber() <= b_end)
            {
                cursor.setPosition(tb.position());

                 cursor.insertText("\t");


                tb = tb.next();
            }
        }
        else
        {
            while(tb.blockNumber() <= blockCount() && tb.isValid()  && tb.blockNumber() <= b_end)
            {
                if(toPlainText()[tb.position()] == '\t')
                {
                    cursor.setPosition(tb.position());
                    cursor.deleteChar();
                }

                tb = tb.next();
            }
        }

        cursor.endEditBlock();
        return;
    }

    //decide if its a real word, or a tab symbol needs to be placed
    if((cursor.position()-1>=0 && (text[cursor.position()-1]=='\n' ||text[cursor.position()-1]=='\t' ||text[cursor.position()-1]=='\r')) || cursor.atBlockStart() || (lasttext.size()<=1 && (lasttext.size() > 0 && lasttext[0]!='.'&& lasttext[0]!='='&& lasttext[0]!='*'&& lasttext[0]!='\''&& lasttext[0]!='(' && lasttext[0]!=')')))
    {
        cursor.insertText("\t");
        return;
    }

    bool lasttexttablecolumn = lastwordisTableColumn;
    QString word = "";

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

        if(cursor.position()-1>=0 && (text[cursor.position()-1]==' '))
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

    int counter = 0;
    for(int i=0;i < cursor.block().text().size();i++)
    {
        qDebug() << cursor.block().text()[i];
        if(cursor.block().text()[i]==' ' || cursor.block().text()[i]=='\t' || cursor.block().text()[i]=='\n' || cursor.block().text()[i]=='\r' || cursor.block().text()[i]=='\x9')
            continue;
        else if(cursor.block().text()[i]=='-')
        {
            counter ++;
            if(counter==2)
            {
                uncoment = true;
                break;
            }
        }
        else
        {
            uncoment = false;
            break;
        }


    }



    QTextBlock tb = cursor.block();

    cursor.beginEditBlock();
    if(uncoment)
    {
        while(tb.blockNumber() <= blockCount() && tb.isValid()  && tb.blockNumber() <= b_end)
        {
            counter = 0;

            for(int i=0;i < tb.text().size();i++)
            {

                qDebug() << tb.text()[i];
                if(tb.text()[i]==' ' || tb.text()[i]=='\t' || tb.text()[i]=='\n' || tb.text()[i]=='\r' || tb.text()[i]=='\x9')
                {
                    continue;
                }
                else if(tb.text()[i]=='-' )
                {
                    counter ++;
                }
                else
                {
                    break;
                }
                if(counter==2)
                {
                    if(tb.position() + i -1 >= 0)
                    {
                        cursor.setPosition(tb.position() + i -1);
                        cursor.deleteChar();
                        cursor.deleteChar();
                    }
                    break;
                }
            }



            tb = tb.next();
        }
    }
    else
    {
        while(tb.blockNumber() <= blockCount() && tb.isValid()  && tb.blockNumber() <= b_end)
        {
            counter = 0;

            cursor.setPosition(tb.position());
            cursor.insertText("--");



            tb = tb.next();
        }
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
static QStringList strl;
void CodeEditor::updateMisc()
{

    qDebug() << "Opening code editors DS";
    // user theme
    if(userDS.Load((documentsDir + "/userdata.txt")))
    {
        userDS.data["UserTheme"]["CodePreviewLineCount"];
        userDS.data["UserTheme"]["CodePreviewAntialiasing"];
        userDS.data["UserTheme"]["CodePreviewSnapCount"];
        userDS.data["UserTheme"]["CodePreview"];
        userDS.data["UserTheme"]["Font"];


        qDebug() << "in";
        QFont fnt = this->font();

        if(userDS.data["UserTheme"]["Font"].size()>2)
            fnt.setFamily(QString(userDS.data["UserTheme"]["Font"]).trimmed());

        fnt.setPointSize(userDS.GetPropertyAsInt("UserTheme","FontSize"));
        this->setFont(fnt);

        qDebug() << "in 2";
        strl = userDS.data["UserTheme"]["Color_BracketHighlight"].split(',');
        qDebug() << "in 3";
        QColor col = QColor(Qt::GlobalColor::darkRed).lighter(35);
        for(int i=0;i<strl.size();i++)
        {
            if(i==0) col.setRed(QVariant(strl[i]).toInt());
            if(i==1) col.setGreen(QVariant(strl[i]).toInt());
            if(i==2) col.setBlue(QVariant(strl[i]).toInt());
            if(i==3) col.setAlpha(QVariant(strl[i]).toInt());
        }
        braccketHighlightColor = col;
        qDebug() << "in 4";


        if(userDS.GetProperty("UserTheme","CodePreview") == "true")
        {
            b_codePreview = true;
        }
        else
            b_codePreview = false;

        userDS.Save((documentsDir + "/userdata.txt"));
        strl.clear();
        qDebug() << "out";
    }
    else
    {
        qDebug() << "file not opened userdata.txt in code editor";
    }

    //Scroll bar setup
    highlighter->updateMisc();
    highlighter->rehighlight();
}


