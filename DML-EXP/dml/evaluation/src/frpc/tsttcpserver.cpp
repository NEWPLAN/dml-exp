#include "rpctcpserver.h"
#include "msgstub.h"
#include "publicfun/public.h"
#include "publicfun/filesystemwrapper.h"

#define LOG_TAG "TstTcpServer"
#include "publicfun/debug.h"

class Tstclientp:public RpcClientProxy{

    int dealreq(RpcProtHead * req)
    {

        req->mHead.cmdtype = EMsgCmdType_Rsp;


        WriteToFile("b.data", req->data.c_str(), req->data.size());
        req->data = "ok";
        return 0;
    }
};

class TstTcpServer:public TcpServer{

    ClientInfo * newClient() override{
        return new Tstclientp;
    }

};



int main()
{
    d_init_debug("", 0, 0, DEBUG_VERBOSE, DEBUG_NONE, "tstserver");

    TstTcpServer tcpserver;
    tcpserver.Start("127.0.0.1", 9999);

    //tcpclient.sendRaw("asfsadf");


    return 0;
}

