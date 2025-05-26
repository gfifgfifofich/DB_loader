#ifndef PATTERNS_H
#define PATTERNS_H

// loading patterns from a separate .cpp file, fro faster compilation

#include <QString>
#include <QVector>
#include <QMap>

inline QVector<QString> keywordPatterns;
inline QVector<QString> subCommandPatterns;

inline QVector<QString> SQLitePatterns;
inline QVector<QString> OraclePatterns;
inline QVector<QString> PostgrePatterns;

inline QString stockUserDataTXT;

void fillPaterns();
#endif // PATTERNS_H
