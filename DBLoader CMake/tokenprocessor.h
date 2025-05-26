#ifndef TOKENPROCESSOR_H
#define TOKENPROCESSOR_H

/*
Processes code/scripts into a markow chain style data.
resulting file contains token sets and probabilities of what can be the next token
*/

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
