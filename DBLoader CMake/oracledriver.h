
#ifdef Oracle_OCI_Driver

#ifndef ORACLEDRIVER_H
#define ORACLEDRIVER_H


#include <qdebug.h>
#include <qlogging.h>
#include "tabledata.h"



#define QOCI_DYNAMIC_CHUNK_SIZE 65535
#define QOCI_PREFETCH_MEM  10240

// setting this define will allow using a query from a different
// thread than its database connection.
// warning - this is not fully tested and can lead to race conditions
#define QOCI_THREADED

#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
enum { QOCIEncoding = 2002 }; // AL16UTF16LE
#else
enum { QOCIEncoding = 2000 }; // AL16UTF16
#endif

#include "occi.h"



class OracleDriver
{
private:

public:

    oracle::occi::Environment *env = nullptr;
    oracle::occi::Connection *con = nullptr;
    oracle::occi::Statement *stmt = nullptr;
    OCIServer* lastserv;

    QString lastError = "";
    OracleDriver();
    void Create(std::string user, std::string passwd, std::string db);
    ~OracleDriver();
    bool runSelect (std::string sqlStmt, TableData* output);
    void cancel();



};



/*

QString 10
QDateTime 16

*/

inline int detectType(int oracle_type)
{
    //mtl[0].getAttributeType(1)
    int type = -1;

    switch (oracle_type) {
    case SQLT_STR:
    case SQLT_VST:
    case SQLT_CHR:
    case SQLT_AFC:
    case SQLT_VCS:
    case SQLT_AVC:
    case SQLT_RDD:
    case SQLT_LNG:
#ifdef SQLT_INTERVAL_YM
    case SQLT_INTERVAL_YM:
#endif
#ifdef SQLT_INTERVAL_DS
    case SQLT_INTERVAL_DS:
#endif
        type = QMetaType::QString;
        qDebug() << "QString" << type;
        break;
    case SQLT_INT:
        type = QMetaType::Double;
        qDebug() << "Int" << type;
        break;
    case SQLT_FLT:
        type = QMetaType::Double;
        qDebug() << "Float" << type;
        break;
    case SQLT_NUM:
        type = QMetaType::Double;
        qDebug() << "QString 1" << type;

        break;
    case SQLT_VNU:
        type = QMetaType::Double;
        qDebug() << "QString 2" << type;
    case SQLT_UIN:
        type = QMetaType::Double;
        qDebug() << "QString 3" << type;
        break;
    case SQLT_VBI:
    case SQLT_BIN:
    case SQLT_LBI:
    case SQLT_LVC:
    case SQLT_LVB:
    case SQLT_BLOB:
    case SQLT_CLOB:
    case SQLT_FILE:
    case SQLT_NTY:
    case SQLT_REF:
    case SQLT_RID:
        type = QMetaType::QByteArray;
        qDebug() << "QByteArray" << type;
        break;
    case SQLT_DAT:
    case SQLT_ODT:
    case SQLT_TIMESTAMP:
    case SQLT_TIMESTAMP_TZ:
    case SQLT_TIMESTAMP_LTZ:
        type = QMetaType::QDateTime;
        qDebug() << "QDateTime" << type;
        break;
    default:
        qDebug() << "warning, oracle column type out of switch";
        break;
    }
    return type;
}

inline QDateTime fromOCIDateTime(oracle::occi::Date dateTime)
{
    int year;
    unsigned int month, day, hour, minute, second;
    unsigned int nsec = 0;
    //int tzHour, tzMinute;// will be modified if needed

    dateTime.getDate(year,month,day,hour,minute,second);

    //OCIDateTimeGetDate(env, err, dateTime, &year, &month, &day);
    //OCIDateTimeGetTime(env, err, dateTime, &hour, &minute, &second, &nsec);
    //OCIDateTimeGetTimeZoneOffset(env, err, dateTime, &tzHour, &tzMinute);
    //int secondsOffset = (qAbs(tzHour) * 60 + tzMinute) * 60;
    //if (tzHour < 0)
    //    secondsOffset = -secondsOffset;
    // OCIDateTimeGetTime gives "fractions of second" as nanoseconds
    return QDateTime(QDate(year, month, day), QTime(hour, minute, second, nsec / 1000000));
}
inline QString fromOCIDateTimeToString (oracle::occi::Date dateTime)
{
    int year;
    unsigned int month, day, hour, minute, second;
    unsigned int nsec = 0;
    //int tzHour, tzMinute;// will be modified if needed

    dateTime.getDate(year,month,day,hour,minute,second);

    QString stry = std::to_string(year).c_str();
    QString strm = std::to_string(month).c_str();
    QString strd = std::to_string(day).c_str();
    QString strh = std::to_string(hour).c_str();
    QString strmm = std::to_string(minute).c_str();
    QString strs = std::to_string(second).c_str();

    while(stry.size() < 4)
        stry = "0" + stry;

    while(strm.size() < 2)
        strm = "0" + strm;

    while(strd.size() < 2)
        strd = "0" + strd;

    while(strh.size() < 2)
        strh = "0" + strh;

    while(strmm.size() < 2)
        strmm = "0" + strmm;

    while(strs.size() < 2)
        strs = "0" + strs;
    return stry + "-" + strm + "-" + strd + " " + strh + ":" + strmm + ":" + strs;
}

#endif // ORACLEDRIVER_H

#endif
