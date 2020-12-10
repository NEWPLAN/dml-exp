#include "tcpclient.h"
#include <unistd.h>

#include "msgstub.h"
#include "publicfun/public.h"
#include "publicfun/filesystemwrapper.h"

#define LOG_TAG "tcptest"
#include "publicfun/debug.h"


class TstMsg:public MsgStub{

    int _dealResponse(RpcProtHead * resp) override
    {
        LOGD("%s E");
        LOGD("%d-%d-%s", resp->mHead.seq, resp->data.size(), resp->data.c_str());
        return 0;
    }

    int _makeRequest() override
    {
        ReadFromFile("/home/firear/temp/netforward", data);
        return 0;
    }

};


int main()
{
    d_init_debug("", 0, 0, DEBUG_VERBOSE, DEBUG_NONE, "tsttcp");

    TcpClient tcpclient;
    tcpclient.connectToServer("127.0.0.1", 9999);

    //tcpclient.sendRaw("asfsadf");
    //usleep(1000);
    TstMsg *msg = new TstMsg;

    tcpclient.syncCall(msg);


	sleep(1);
    return 0;
}

