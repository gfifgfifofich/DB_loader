#ifndef PATTERNS_H
#define PATTERNS_H

#include <QString>
#include <QVector>
#include <QMap>

inline QVector<QString> keywordPatterns;

inline QVector<QString> SQLitePatterns;
inline QVector<QString> OraclePatterns;
inline QVector<QString> PostgrePatterns;
void fillPaterns();
#endif // PATTERNS_H
