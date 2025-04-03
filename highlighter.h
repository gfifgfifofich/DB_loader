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
struct S_TextInterval
{
    int start = 0;
    int end = 0;
    QString text = "";
};
inline QVector<TextInterval> textintervals;



class Highlighter : public QSyntaxHighlighter
{
    Q_OBJECT

private:
    int prevBlockCount = false;
    int currentBlockCount  = false;
    int BlockCountDiff  = false;
    bool newBlockCount = false;
public:
    Highlighter(QTextDocument *parent = 0);

    QMap<QString,bool> ColumnMap;
    QMap<QString,QString> TableColumnAliasMap; // key-alias-name / key-table-name
    QMap<QString,QMap<QString,bool>> tmpTableColumnMap;
    QMap<int,QMap<QString,QMap<QString,bool>>> tmpTableColumnMapPerRow;
    QMap<QString,QMap<QString,bool>> TableColumnMap;

    QVector<QString> dbPatterns;

    QVector<QString> SQLitedbPatterns;
    QVector<QString> OracledbPatterns;
    QVector<QString> PostgredbPatterns;

    QMap<QString,bool> ColumnMap_lower;
    QMap<QString,QString> TableColumnAliasMap_lower; // key-alias-name / key-table-name
    QMap<QString,QMap<QString,bool>> TableColumnMap_lower;

    // tokens in current sql query, from ; to ; or from 0 to end
    QMap<int,QVector<S_TextInterval>> tokens;
    QMap<int,QMap<QString,TextInterval>> TableAliasMapPerRow;



    bool PostgresStyle = false;
    bool QSLiteStyle  = false;
    QString dbSchemaName = "";

    DataStorage TableColumnDS;
    void UpdateTableColumns(QSqlDatabase* db, QString dbname);

    void HighLightALl();

    void OnBlockCountChanged(int newBlockCount) ;
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
