#ifndef HIGHLIGHTER_H
#define HIGHLIGHTER_H
#include "datastorage.h"
#include <QObject>
#include <QSyntaxHighlighter>
#include <QRegularExpression>
#include <qsqldatabase.h>

struct TextInterval
{
    int start = 0;
    int end = 0;
    QString PrevWord = "";
    QString text = "";
    int type = 0;
    bool is_number = false;
};
inline QVector<TextInterval> textintervals;

class Highlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    Highlighter(QTextDocument *parent = 0);

    QMap<QString,bool> ColumnMap;
    QMap<int,QMap<QString,TextInterval>> TableAliasMapPerRow;
    QMap<QString,QString> TableColumnAliasMap; // key-alias-name / key-table-name

    QVector<QString> dbPatterns;
    QMap<QString,QMap<QString,bool>> TableColumnMap;

    DataStorage TableColumnDS;
    void UpdateTableColumns(QSqlDatabase* db);

    void HighLightALl();

protected:
    void highlightBlock(const QString &text) override;

private:
    struct HighlightingRule
    {
        QRegularExpression pattern;
        QTextCharFormat format;
    };
    QVector<HighlightingRule> highlightingRules;


    QRegularExpression commentStartExpression;
    QRegularExpression commentEndExpression;

    QTextCharFormat keywordFormat;
    QTextCharFormat NameFormat;
    QTextCharFormat classFormat;
    QTextCharFormat singleLineCommentFormat;
    QTextCharFormat multiLineCommentFormat;
    QTextCharFormat quotationFormat;
    QTextCharFormat functionFormat;
};
#endif // HIGHLIGHTER_H
