#ifndef SQLSUBFUNCTIONS_H
#define SQLSUBFUNCTIONS_H

/*
Functions for easier format translation & stuff
*/

#include <qdatetime.h>
#include <qobject.h>
#include <qvariant.h>

inline bool isSpecialSymbol(QChar c)
{
    return c=='!' || c=='=' || c=='*' || c=='&' || c=='%' || c=='(' || c==')' || c=='-' || c=='+' || c=='/' || c==',' || c=='.' || c=='<' || c=='>' || c==';' || c==':' || c=='\"' || c=='\'';
}
inline bool isWord(QString s)
{
    return !s.contains('!') &&
           !s.contains('=') &&
           !s.contains('*') &&
           !s.contains('&') &&
           !s.contains('%') &&
           !s.contains('(') &&
           !s.contains(')') &&
           !s.contains('-') &&
           !s.contains('+') &&
           !s.contains('/') &&
           !s.contains(',') &&
           !s.contains('.') &&
           !s.contains('<') &&
           !s.contains('>') &&
           !s.contains(';') &&
           !s.contains(':') &&
           !s.trimmed().contains(' ');
}
inline bool HasLetters(QString s)
{
    return (s.count('!') +
           s.count('=') +
           s.count('*') +
           s.count('&') +
           s.count('%') +
           s.count('(') +
           s.count(')') +
           s.count('-') +
           s.count('+') +
           s.count('/') +
           s.count(',') +
           s.count('.') +
           s.count('<') +
           s.count('>') +
           s.count(';') +
           s.count(':') +
           s.count('0') +
           s.count('1') +
           s.count('2') +
           s.count('3') +
           s.count('4') +
           s.count('5') +
           s.count('6') +
           s.count('7') +
           s.count('8') +
           s.count('9') +
           s.count('e') +
           s.count('E') +
           s.count(' ')) < s.size() || s.count('E')>1|| s.count('e')>1;
}
inline bool isNumber(QString s)
{
    if(s.count('.') + s.count(',')>1)
        return false;
    for(auto x : s)
    {
        if(!x.isDigit() && x!='.'&& x!=','&& x!='e'&& x!='E')
            return false;
    }
    return true;
}
// used mostly to not highlight window functions after comma as errors
inline bool isWindowFunc(QString s)
{
    QString str = s.toLower().trimmed();
    return str == "coalesce" ||
           str == "case" ||
           str == "trunc" ||
           str == "date" ||
           str == "lag" ||
           str == "lead" ||
           str == "first" ||
           str == "first_value" ||
           str == "last_value" ||
           str == "(" ||
           str == "sum" ||
           str == "count" ||
           str == "max" ||
           str == "min" ||
           str == "avg"||
           str == "round"||
           str == "last_day"||
           str == "to_char"||
           str == "substr"||
           str == "lower"||
           str == "upper"||
           str == "nvl"||
           str == "account"||
           str == "sysdate"||
           str == "priority"||
           str == "concat"


        ;
}

inline QStringList processBlockToTokens(QString& text)
{

    QStringList tokens;
    int wordstart = 0;
    int wordend = 0;
    bool inword = false;
    QString word = "";
    for(int i =0;i<text.size();i++)
    {
        // word handling
        if(text[i] != ' ' && text[i] != '\n' && text[i] != '\t' && !isSpecialSymbol(text[i]))
        {
            if(!inword)
                wordstart = i;
            word += text[i];
            inword = true;
        }
        else if (inword)
        {
            inword = false;
            wordend = i;
            tokens.push_back(word);
            if(text[i] == '\n')
                tokens.push_back("\n");
            word.clear();
        }

        //Special symbol handling
        if(isSpecialSymbol(text[i]) && i+1 < text.size() && isSpecialSymbol(text[i+1]) ) //this and next
        {
            QString tword = text[i];
            tword += text[i+1];
            tokens.push_back(tword);
            inword = false;
            word.clear();
            i++;// hop to the next unprocessed symbol
        }
        else if(isSpecialSymbol(text[i])) // only this
        {
            QString tword = text[i];
            tokens.push_back( tword );
            word.clear();
            inword = false;
        }
    }
    if(inword)
    {
        inword = false;
        wordend = text.size();
        tokens.push_back(word);
        word.clear();
    }
    return tokens;
}

inline QVariant fixQVariantTypeFormat(QVariant var)
{

    if(var.typeId() == 10) // additional type checking on strings
    {
        QString str = var.toString();
        QDateTime dt = var.toDateTime();
        if(dt.isValid() && !dt.isNull())
        {// datetime
            return QVariant(dt);
        }
        var = str;
        bool hascomas = false;
        bool isdouble = false;
        bool force_double = false;
        bool isint = false;
        bool forcetext = false;
        double doub = var.toDouble(&isdouble);
        var = str;
        int integ = var.toInt(&isint);
        var = str;
        // passes through values like 12312.123123213123123123
        // fix implemented, currently testing
        if(str.size()>15 || (!str.startsWith("0.") || !str.startsWith("0,")) && str.startsWith("0") || str.startsWith("+"))
        {
            doub = false;
            isint = false;
        }
        else if(str.count('.') + str.count(',') == 1 && !((!str.startsWith("0.") || !str.startsWith("0,")) && str.startsWith("0")))
        {// one coma/dot

            QStringList strl = str.replace('.',',').split(',');

            if(str.contains(','))
                hascomas = true;

            if(isNumber(str))
            {
                force_double = true;
            }
            strl = str.replace(',','.').split('.');

            if(strl.size()==2)
            {
                bool ok1 = false;
                bool ok2 = false;
                QVariant(strl[0]).toInt(&ok1);
                QVariant(strl[1]).toInt(&ok2);
                if(ok1 && ok2)
                    force_double = true;
            }
        }
        if(force_double )
        {
            if(!hascomas)
                return QVariant(var.toDouble());
            else
                return QVariant((var.toString().replace(',','.')).toDouble());
        }
        else
        {
            if((!dt.isValid() || dt.isNull()) && isdouble && str.size()>9)
            {
                isdouble = false;
                forcetext = true;
            }
            if(!forcetext)
            {

                if(dt.isValid() && !dt.isNull())
                {// datetime
                    return QVariant(dt);
                }
                else if(isdouble && str.count('.') + str.count(',') > 0)
                {
                    return QVariant(var.toDouble());
                }
                else if(isint)
                {
                    return QVariant(var.toLongLong());
                }
            }
            else
                return str;
        }
    }
    return var;
}

thread_local inline int fixQStringType_lasttype = 10;
inline QString fixQStringType(QString str)
{

    fixQStringType_lasttype = 10;

    if((str.size()>=2 && !(str[0]==QChar('0') && str[1]!=QChar('.')&& str[1]!=QChar(',')) && str[0]!=QChar('+')) || str.size()< 2)
    {
        int cnt = str.count('.');
        bool isdouble = false;
        if(cnt == 1)
        {
            str.toDouble(&isdouble);
            if(isdouble)
            {
                fixQStringType_lasttype = 6;
                return str;
            }
        }
        if (cnt > 1)
        {

            if(str.size() == 19 || str.size() == 23 || str.size() == 10)//2000-01-01 10:10:10 || 2000-01-01 10:10:10.000 || 2000-01-01
            {
                if((str[4] == '-' || str[4] == '.') && (str[7] == '-' || str[7] == '.'))
                {
                    fixQStringType_lasttype = 16; // probably is date
                    if(str.size()<19)//its only date;
                    {
                        str.resize(19);
                        str[10] = ' ';
                        str[11] = '0';
                        str[12] = '0';
                        str[13] = ':';
                        str[14] = '0';
                        str[15] = '0';
                        str[16] = ':';
                        str[17] = '0';
                        str[18] = '0';
                    }
                    else
                        str.resize(19);

                    return str.replace('T',' ');
                }
            }
            return str;
        }
        cnt+= str.count(',');

        if(cnt == 1)
        {
            QString doub = str.replace(',','.');
            doub.toDouble(&isdouble);
            if(isdouble)
            {
                fixQStringType_lasttype = 6;
                return str;
            }
        }
        if (cnt <= 0)
        {
            bool ok = false;
            str.toInt(&ok);

            if(ok)
            {
                fixQStringType_lasttype = 6;
                return str;
            }
        }

        if(str.size() == 19 || str.size() == 23 || str.size() == 10)//2000-01-01 10:10:10 || 2000-01-01 10:10:10.000 || 2000-01-01
        {
            if((str[4] == '-' || str[4] == '.') && (str[7] == '-' || str[7] == '.'))
            {
                fixQStringType_lasttype = 16; // probably is date
                if(str.size()<19)//its only date;
                {
                    str.resize(19);
                    str[10] = ' ';
                    str[11] = '0';
                    str[12] = '0';
                    str[13] = ':';
                    str[14] = '0';
                    str[15] = '0';
                    str[16] = ':';
                    str[17] = '0';
                    str[18] = '0';
                }
                else
                    str.resize(19);

                return str.replace('T',' ');
            }
        }
        return str;
    }
    else
    {// string or date
        if(str.size() == 19 || str.size() == 23 || str.size() == 10)
        {
            if((str[4] == '-' || str[4] == '.') && (str[7] == '-' || str[7] == '.'))
            {
                fixQStringType_lasttype = 16; // probably is date
                if(str.size()<19)//its only date;
                {


                    str.resize(19);
                    str[10] = ' ';
                    str[11] = '0';
                    str[12] = '0';
                    str[13] = ':';
                    str[14] = '0';
                    str[15] = '0';
                    str[16] = ':';
                    str[17] = '0';
                    str[18] = '0';
                }
                else
                    str.resize(19);

                return str.replace('T',' ');
            }
        }
        return str;
    }
    return str;
}




#include <QtCore>

class BPE {
public:
    // Ordered list of merges (A,B) learned during training.
    QList<QPair<QString, QString>> merges;

    // Train BPE on a corpus (whitespace tokenized words), performing numMerges merges.
    void train(const QString& corpus, int numMerges) {
        // 1) Build word frequency map (whitespace split).
        QHash<QString, int> wordFreq;
        const auto words = corpus.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
        for (const QString& w : words) wordFreq[w]++;

        // 2) Convert to vocabulary: list of (tokenizedSymbols, freq)
        QList<QPair<QStringList, int>> vocab = toVocab(wordFreq);

        // 3) Iteratively merge the most-frequent adjacent pair.
        for (int i = 0; i < numMerges; ++i) {
            QHash<QString, qint64> pairCounts = countPairs(vocab);
            if (pairCounts.isEmpty()) break;

            // Find best pair by count
            QString bestKey;
            qint64 bestCount = -1;
            for (auto it = pairCounts.constBegin(); it != pairCounts.constEnd(); ++it) {
                if (it.value() > bestCount) {
                    bestCount = it.value();
                    bestKey = it.key();
                }
            }
            if (bestCount <= 0 || bestKey.isEmpty()) break;

            // Decode "A B" -> (A,B)
            const int sp = bestKey.indexOf(QLatin1Char(' '));
            const QString A = bestKey.left(sp);
            const QString B = bestKey.mid(sp + 1);

            merges.append(qMakePair(A, B));

            // Apply merge to all words in vocab
            for (auto& wf : vocab) {
                mergeSymbols(wf.first, A, B);
            }
        }
    }

    // Encode arbitrary text: returns all tokens as a QStringList.
    // Splits text to words on whitespace, applies learned merges per word.
    QStringList encode(const QString& text) const {
        QStringList tokens;
        const auto words = text.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
        for (const QString& w : words) {
            QStringList symbols = wordToSymbols(w);
            // Apply merges in learned order
            for (const auto& m : merges) {
                mergeSymbols(symbols, m.first, m.second);
            }
            tokens.append(symbols);
        }
        return tokens;
    }

    // Decode tokens produced by encode() back to text.
    QString decode(const QStringList& tokens) const {
        QStringList wordsOut;
        QString current;
        for (const QString& t : tokens) {
            if (t.endsWith("</w>")) {
                QString piece = t;
                piece.chop(4); // remove "</w>"
                current += piece;
                wordsOut.append(current);
                current.clear();
            } else {
                current += t;
            }
        }
        if (!current.isEmpty()) {
            // If a final word didn't end with </w>, still emit it.
            wordsOut.append(current);
        }
        return wordsOut.join(QLatin1Char(' '));
    }

private:
    // Turn a word into initial symbols (characters) + "</w>"
    static QStringList wordToSymbols(const QString& word) {
        QStringList symbols;
        symbols.reserve(word.size() + 1);
        for (QChar ch : word) {
            symbols.append(QString(ch));
        }
        symbols.append(QStringLiteral("</w>"));
        return symbols;
    }

    // Build vocabulary: list of (symbols, frequency) from word frequency map.
    static QList<QPair<QStringList, int>> toVocab(const QHash<QString, int>& wordFreq) {
        QList<QPair<QStringList, int>> vocab;
        vocab.reserve(wordFreq.size());
        for (auto it = wordFreq.constBegin(); it != wordFreq.constEnd(); ++it) {
            vocab.append(qMakePair(wordToSymbols(it.key()), it.value()));
        }
        return vocab;
    }

    // Count adjacent pair frequencies across vocab (weighted by word freq).
    static QHash<QString, qint64> countPairs(const QList<QPair<QStringList, int>>& vocab) {
        QHash<QString, qint64> counts;
        for (const auto& wf : vocab) {
            const QStringList& syms = wf.first;
            const int freq = wf.second;
            for (int i = 0; i + 1 < syms.size(); ++i) {
                const QString key = pairKey(syms.at(i), syms.at(i + 1));
                counts[key] += freq;
            }
        }
        return counts;
    }

    // Merge all occurrences of adjacent pair (A,B) inside symbols.
    static void mergeSymbols(QStringList& symbols, const QString& A, const QString& B) {
        if (symbols.size() < 2) return;
        for (int i = 0; i + 1 < symbols.size();) {
            if (symbols.at(i) == A && symbols.at(i + 1) == B) {
                symbols[i] = A + B;
                symbols.removeAt(i + 1); // merged; stay on same i to catch cascading merges
            } else {
                ++i;
            }
        }
    }

    static QString pairKey(const QString& a, const QString& b) {
        // No spaces can appear inside tokens in this char-level scheme, so space is safe.
        return a + QLatin1Char(' ') + b;
    }
};


#endif // SQLSUBFUNCTIONS_H
