#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <map>
#include <string>
#include "msgstub.h"
#include "publicfun/condmutex.h"


using namespace std;


#define MAX_WAIT_QUEUE_SIZE 32


enum SessionState{
    INIT,
    CONNECTED,
    LOGINED
};



/**
 * @todo write docs
 */
class TcpClient
{
public:
    /**
     * Default constructor
     */
    TcpClient();

    /**
     * Destructor
     */
    ~TcpClient();

    int connectToServer(const char *addr, const uint16_t port);

    int sendRaw(const string &data);

    int syncCall(MsgStub *msg);
    int aSyncCall(MsgStub *msg);

private:
    int dorecv(string &recvbuf);
    int dosend(MsgStub &rpcjmsg);
    void handleSocketClose();
    int dispatchMsg(string &msg);

    int insertWaitNode(MsgStub *msg);

    bool mClose;
    int mSocket;
    SessionState mState;

    string mServerAddr;
    int mServerPort;

    map<uint16_t, MsgStub *> mRespMap; //seq
    int close(bool detach = false);

    CondMutex mRespMutexCond;
    CondMutex mSockMutex;

    int recvDataPreProcess(RpcProtHead* &recvnode, string &recvstring);
    int processHeartBeatMsgData(string &msg);

    static int serverRecvLoop(int , void *argv[]);
    static int dealResponse(int argc, void *argv[]);
    uint32_t mSeq;
};

#endif // TCPCLIENT_H
