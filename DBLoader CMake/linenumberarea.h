#ifndef LINENUMBERAREA_H
#define LINENUMBERAREA_H

#include <QWidget>
#include "codeeditor.h"

class LineNumberArea : public QWidget
{
public:
    LineNumberArea(CodeEditor *editor) : QWidget(editor), codeEditor(editor)
    {}
    int id = 0;

    QSize sizeHint() const override
    {
        return QSize(codeEditor->lineNumberAreaWidth(), 0);
    }

protected:
    void paintEvent(QPaintEvent *event) override
    {
        if(id==0)
            codeEditor->lineNumberAreaPaintEvent(event);
        else if (id==1)
            codeEditor->drawPreview(event);

    }

private:
    CodeEditor *codeEditor;
};

#endif // LINENUMBERAREA_H
