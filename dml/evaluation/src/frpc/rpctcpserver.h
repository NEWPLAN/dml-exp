#ifndef RPCTCPSERVER_H
#define RPCTCPSERVER_H
#include "tcpserver.h"
#include "rpchead.h"
#include "encyptwrapper/crc32.h"

class RpcClientProxy : public ClientInfo
{

public:
    int processHeartBeatMsgData(std::string & /*msg*/);
    int ProcessMsgBeforeSend(RpcProtHead *rpcjmsg, string &resp);
    int dealClientData(std::__cxx11::string &cdata, std::__cxx11::string &resp) override;

    //current msg
    //return -1:a error msg;
    //return 0 no more msg;
    //return 1.ok
    int recvDataPreProcess(RpcProtHead *&recvHead, string &recvstring);

protected:
    virtual int dealreq(RpcProtHead *req) = 0;
};

#endif // RPCTCPSERVER_H
