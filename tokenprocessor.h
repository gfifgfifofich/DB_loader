#ifndef TOKENPROCESSOR_H
#define TOKENPROCESSOR_H

#include <QObject>
#include "datastorage.h"
#include <QMap>

class TokenProcessor : public QObject
{
    Q_OBJECT
public:
    explicit TokenProcessor(QObject *parent = nullptr);

    QStringList tokens;
    QMap<QString, QString> uniqueTokens;
    DataStorage ds;
    void processText(QString &text);
    void addFrequencies();
signals:
};
inline TokenProcessor tokenProcessor;
#endif // TOKENPROCESSOR_H
