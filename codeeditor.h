#ifndef CODEEDITOR_H
#define CODEEDITOR_H
#include <QObject>
#include <QPlainTextEdit>
#include "highlighter.h"

class CodeEditor : public QPlainTextEdit
{
    Q_OBJECT

public:
    CodeEditor(QWidget *parent = nullptr);

    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();
    Highlighter* highlighter = nullptr;

    QString lastSuggestedWord=" ";
    bool lastwordisTableColumn = false;

    int prevTabPos = 0;
    int prevTabTextSize;

    void FillsuggestName();
protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void highlightCurrentLine();
    void updateLineNumberArea(const QRect &rect, int dy);

    void suggestName();
    void fillName();

    void CommentSelected();

signals:
    void s_suggestedName();

    void tabDetected();

private:
    QWidget *lineNumberArea;
};

#endif // CODEEDITOR_H
