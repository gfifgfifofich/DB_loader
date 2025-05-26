

#ifdef Oracle_OCI_Driver


#include "oracledriver.h"
#include "sqlSubfunctions.h"
#include <qtimezone.h>

OracleDriver::OracleDriver()
{

}
void OracleDriver::Create(std::string user, std::string passwd, std::string db)
{
    env = oracle::occi::Environment::createEnvironment ( "CL8MSWIN1251", "CL8MSWIN1251",oracle::occi::Environment::Mode::DEFAULT);
    con = env->createConnection (user, passwd, db);
    lastserv = con->getOCIServer();
}

OracleDriver::~OracleDriver()
{
    if(env!= nullptr && con != nullptr)
        env->terminateConnection (con);
    if(env!= nullptr)
        oracle::occi::Environment::terminateEnvironment (env);
}
bool OracleDriver::runSelect (std::string sqlStmt, TableData* output)
{
    bool status = true;
    try {
        stmt = con->createStatement (sqlStmt);
        lastserv = con->getOCIServer();

        oracle::occi::ResultSet *rset = stmt->executeQuery ();
        stmt->setPrefetchRowCount(5000); // enable bulk collect
        //stmt->setPrefetchMemorySize(100*1024*1024); // enable bulk collect, 100 mb

        std::vector<oracle::occi::MetaData> mtl =rset->getColumnListMetaData();


        std::vector<int> types;
        int type = 0;
        qDebug() << mtl.size();
        types.resize(mtl.size());
        output->headers.clear();
        output->headers.resize(mtl.size());
        output->tbldata.clear();
        output->tbldata.resize(mtl.size());
        for(int i=0;i < mtl.size(); i++)
        {
            types[i] = detectType(mtl[i].getInt(oracle::occi::MetaData::ATTR_DATA_TYPE));
            output->headers[i] = mtl[i].getString(oracle::occi::MetaData::ATTR_NAME).c_str();
        }



        int ccnt = 0;
        while (rset->next () && mtl.size() > 0)
        {
            for(int i=0;i < mtl.size(); i ++)
                if(types[i]!=16)
                {
                    //qDebug() << QString().fromStdString(rset->getString(i+1));
                    output->tbldata[i].push_back(fixQVariantTypeFormat(QString().fromStdString(rset->getString(i+1))));
                }
                else
                    if(!rset->isNull(i + 1))
                    {
                        output->tbldata[i].push_back(fixQVariantTypeFormat(fromOCIDateTime(rset->getDate(i+1))));
                    }
                    else
                    {
                        output->tbldata[i].push_back(fixQVariantTypeFormat(QDateTime()));
                    }
        }

        stmt->closeResultSet (rset);
    }catch(oracle::occi::SQLException ex)
    {
        qDebug()<<"oracle::occi::SQLException at runSelect" << " Error number: "<<  ex.getErrorCode()  << "    "<<ex.getMessage() ;
        lastError = ex.getMessage().c_str();
        status = false;
    }

    con->terminateStatement (stmt);
    stmt = nullptr;
    return status;
}


void OracleDriver::cancel()
{
    if(lastserv!=nullptr)
    {
        qDebug() << "realy sending OCIBreak(con->getOCIServer(),0);";

        static OCIError *errhp = NULL;
        (void) OCIHandleAlloc( (dvoid *) env->getOCIEnvironment(), (dvoid **) &errhp, OCI_HTYPE_ERROR, (size_t) 0, (dvoid **) 0);


        int intResult = OCIBreak(con->getOCIServiceContext(), errhp);
        qDebug() << "intResult = OCI_SUCCESS " << intResult  << " = "<< OCI_SUCCESS << OCI_ERROR << OCI_SUCCESS_WITH_INFO << OCI_INVALID_HANDLE;
        intResult = OCIBreak(con->getOCIServer(), errhp);
        qDebug() << "serv intResult = OCI_SUCCESS " << intResult  << " = "<< OCI_SUCCESS << OCI_ERROR << OCI_SUCCESS_WITH_INFO << OCI_INVALID_HANDLE;

        try
        {
            OCIHandleFree(errhp,OCI_HTYPE_ERROR);
        }
        catch(oracle::occi::SQLException ex)
        {
            qDebug() << ex.getErrorCode() << ex.getMessage();
        }

    }
}
#endif
