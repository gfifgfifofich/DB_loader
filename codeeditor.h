#ifndef CODEEDITOR_H
#define CODEEDITOR_H
#include <QObject>
#include <QPlainTextEdit>
#include "highlighter.h"

class CodeEditor : public QPlainTextEdit
{

    Q_OBJECT
    bool ctrl_pressed = false;

public:
    CodeEditor(QWidget *parent = nullptr);

    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();
    Highlighter* highlighter = nullptr;

    QString lastSuggestedWord=" ";
    bool lastwordisTableColumn = false;

    void replace(int _from, int _to, QString _what, QString _with);
    void FillsuggestName();
    QStringList GetTokensUnderCursor();

    void getScreenshot(QPixmap &map);

    QPixmap zoomOutView;
    bool updateZoomOutViewOnScroll = true;

protected:
    void resizeEvent(QResizeEvent *event) override;
    void keyPressEvent(QKeyEvent *e) override;
    void keyReleaseEvent(QKeyEvent *e) override;

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void updateLineNumberArea(const QRect &rect, int dy);

    void CommentSelected();
    void highlightCurrentLine();
    void suggestName();
    void fillName();

signals:
    void s_suggestedName();
    void tabDetected();

private:
    QWidget *lineNumberArea;
};

#endif // CODEEDITOR_H
