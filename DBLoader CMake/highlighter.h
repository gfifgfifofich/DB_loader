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


class CustomBlockData : public QTextBlockUserData
{
public:
    QString lastText = "";

};


class Highlighter : public QSyntaxHighlighter
{
    Q_OBJECT

private:
    // attempt to rehiglhight only what is needed. Still requeres some fixing
    int prevBlockCount = 0;
    int currentBlockCount  = 0;
    int BlockCountDiff  = 0;
    bool newBlockCount = false;
public:
    Highlighter(QTextDocument *parent = 0);


    int FontSizeOverride = -1;

    // highlihghting formats
    QTextCharFormat keywordFormat;
    QTextCharFormat NameFormat;
    QTextCharFormat classFormat;
    QTextCharFormat singleLineCommentFormat;
    QTextCharFormat multiLineCommentFormat;
    QTextCharFormat quotationFormat;
    QTextCharFormat functionFormat;


    // Token maps for highlighting
    QMap<QString,bool> ColumnMap;
    QMap<QString,QString> TableColumnAliasMap; // key-alias-name / key-table-name
    QMap<QString,QMap<QString,bool>> TableColumnMap;
    // database specific keywords
    QVector<QString> dbPatterns; // current db's keywords
    QVector<QString> SQLitedbPatterns;
    QVector<QString> OracledbPatterns;
    QVector<QString> PostgredbPatterns;
    // lower() versions of token maps
    QMap<QString,bool> ColumnMap_lower;
    QMap<QString,QMap<QString,bool>> TableColumnMap_lower;

    // Current tokens/aliases
    QMap<int,QVector<S_TextInterval>> tokens;
    QMap<int,QMap<QString,TextInterval>> TableAliasMapPerRow;
    QMap<QString,QString> TableColumnAliasMap_lower; // key-alias-name / key-table-name
    QMap<QString,QMap<QString,bool>> tmpTableColumnMap;
    QMap<int,QMap<QString,QMap<QString,bool>>> tmpTableColumnMapPerRow;

    std::vector<std::vector<std::pair<int,int>>> lineInterval;// start/end per line
    std::vector<std::vector<QColor>> lineIntervalColor;
    std::vector<std::vector<QTextCharFormat>> lineIntervalFormats;
    bool updateAllHighlighting = false;

    // database specific highlighting
    bool PostgresStyle = false;
    bool QSLiteStyle  = false;
    QString dbSchemaName = "";

    DataStorage TableColumnDS;
    DataStorage AllTableColumnDS;
    void UpdateTableColumns(QSqlDatabase* db, QString dbname);
    void HighLightALl();
    void OnBlockCountChanged(int newBlockCount) ;

    void updateMisc();


protected:
    void highlightBlock(const QString &text) override;

private:

    // unused regexes
    QRegularExpression commentStartExpression;
    QRegularExpression commentEndExpression;

    struct HighlightingRule
    {
        QRegularExpression pattern;
        QTextCharFormat format;
    };
    QVector<HighlightingRule> highlightingRules;
};
#endif // HIGHLIGHTER_H
