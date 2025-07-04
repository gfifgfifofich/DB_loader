#ifndef CODEEDITOR_H
#define CODEEDITOR_H
#include <QObject>
#include <QPlainTextEdit>
#include "highlighter.h"

class CodeEditor : public QPlainTextEdit
{

    Q_OBJECT
    bool ctrl_pressed = false;
    bool shift_pressed = false;

public:
    CodeEditor(QWidget *parent = nullptr);

    void lineNumberAreaPaintEvent(QPaintEvent *event);
    void drawPreview(QPaintEvent *event);
    void drawSuggestedWord(QPaintEvent *event);


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
    bool b_codePreview = false;

    void updateMisc();

protected:
    void resizeEvent(QResizeEvent *event) override;
    void keyPressEvent(QKeyEvent *e) override;
    void keyReleaseEvent(QKeyEvent *e) override;
public slots:
    void CommentSelected();

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void updateLineNumberArea(const QRect &rect, int dy);

    void highlightCurrentLine();
    void suggestName();
    void fillName();

signals:
    void s_suggestedName();
    void tabDetected();

private:
    QWidget *lineNumberArea;
    QWidget *codePreview;
    QWidget *suggestedWordDrawer;
};

#endif // CODEEDITOR_H
