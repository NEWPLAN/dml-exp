#include "tcpclient.h"

#define LOG_TAG "tcpclient"
#include "publicfun/debug.h"
#include <threadwrapper/threadwrapper.h>
#include <socketwrapper/socketWrapper.h>
#include "publicfun/quicklz/quicklz.h"
#include "encyptwrapper/crc32.h"


TcpClient::TcpClient():
mState(INIT),mSeq(0)
{
    ictnet_init();
}

TcpClient::~TcpClient()
{
    ictnet_release();
}

int TcpClient::serverRecvLoop(int , void *argv[])
{
    int ret = 0;
    string recvStr;

    TcpClient *session = (TcpClient *)argv[0];
    while (!session->mClose){
        ret = session->dorecv(recvStr);
        if(ret < 0){
            LOGD("%s:error:%s\n", __FUNCTION__, strerror(errno));
            session->handleSocketClose();
            break;
        }else if(ret > 0){
            session->dispatchMsg(recvStr);
        }
    }

    return ret;
}



int TcpClient::dorecv(string &recvbuf)
{
    char buf[20480] = "\0";
    int len = ictnet_recv_ex(&mSocket, buf, sizeof(buf), 25000);

    if(len > 0){
        //PrintBuffer(buf, len);
        recvbuf.append(string((const char *)buf, len));
    }
    LOGV("%s x with %d\n", __FUNCTION__, len);

    return len;
}

void TcpClient::handleSocketClose()
{
    //remove all wait node
    this->close(true);
    mRespMutexCond.broadcast();
    mRespMutexCond.lock();
    if(mRespMap.size() != 0){
        map <uint16_t, MsgStub *>::iterator it;
        for(it = mRespMap.begin(); it != mRespMap.end(); it++){
            MsgStub *node = it->second;
            node->dealResponse(NULL);
            if(!node->IsSync()){
                delete node;
            }
            mRespMap.erase(it);
        }
    }
    mRespMutexCond.unlock();
}

int TcpClient::connectToServer(const char *addr, const uint16_t port)
{
    int ret = 0;
    LOGD("%s E\n", __FUNCTION__);
    if(mState >= CONNECTED){
        LOGD("havd connected\n");
        return ret;
    }
    mServerAddr = addr;
    mServerPort = port;
    ictnet_initclient(&mSocket);
    if((ret = ictnet_connect_url(&mSocket, mServerAddr.c_str(), mServerPort)) == 0){
        mState = CONNECTED;
        //create data recv thread;
        mClose = false;
        ThreadWrapper t(serverRecvLoop, 1, this);
        t.detach();
        //thread dataRecvThread();

        //dataRecvThread.join();
        //recvThread = &dataRecvThread;
        LOGD("connect server succeed\n");
    }else{
        LOGE("connect server failed:%s\n", strerror(errno));
        mState = INIT;
        handleSocketClose();
    }
    //todo send msg to msg queue create data send thread,read from q and send to server;
    //but now we just send data to socket.
    return ret;
}

int TcpClient::processHeartBeatMsgData(std::string& /*msg*/)
{
    return 0;
}

int TcpClient::sendRaw(const string &data)
{
    mSockMutex.lock();
    int ret = ictnet_send(&mSocket, data.c_str(), data.length(), data.length());
    mSockMutex.unlock();
    return ret;
}


int TcpClient::aSyncCall(MsgStub *msg)
{
    msg->makeRequest(mSeq++);
    int ret = insertWaitNode(msg);
    if(!ret){
        dosend(*msg);
    }else{
        delete msg;
    }
    return ret;
}

int TcpClient::dosend(MsgStub &rpcjmsg)
{
/*
16B head{
    uint8_t magic; 0xdf;//-33;
    uint8_t version; 0x01
    uint8_t cmdtype; 0 phone->server 1 server -> phone
    uint8_t flag;
    uint16_t headcrc16;
    uint16_t datacrc;
    uint16_t seq;
    uint16_t timeout;
    uint32_t datalen;
};
data{
    4B  method length ml;uint32_t
    mlB method;
    4B  subDataSize sds;uint32_t
    sdsB subData
}
*/

    rpcjmsg.setBreadNetWidth(true);

    char localPackData[16] = "\0";

    CRC32 crc32;
    rpcjmsg.mHead.datacrc = crc32.crc32(rpcjmsg.data.c_str(), rpcjmsg.data.size());
    rpcjmsg.mHead.datalen = rpcjmsg.data.size();

    rpcjmsg.packHead(localPackData);
    LOGI("postMsg seq=%u\n", rpcjmsg.mHead.seq);
    //PrintBuffer(localPackData, datalen);
    mSockMutex.lock();
    int ret = ictnet_send(&mSocket, localPackData, 16, 16);
    if(ret == 16){
        ret = ictnet_send(&mSocket, rpcjmsg.data.c_str(), rpcjmsg.data.size(), rpcjmsg.data.size());
    }
    mSockMutex.unlock();
    //send(mSocket, localPackData, datalen, 0);
    LOGV("%s x with %d\n", __FUNCTION__, ret);

    return ret;
}


int TcpClient::syncCall(MsgStub *msg)
{
    int ret = -1;

    LOGD("%s E\n", __FUNCTION__);
    msg->makeRequest(mSeq++);
    msg->SetSync();
    ret = insertWaitNode(msg);
    if(!ret){
        ret = dosend(*msg);
        if(ret == msg->data.length()){
            if(msg->waitDone()){
                ret = 0;
            }
        }
    }
    if(ret == 0){ // DONE
        delete msg;
    }

    return ret;
}


//current msg
//return -1:a error msg;
//return 0 no more msg;
//return 1.ok
int TcpClient::recvDataPreProcess(RpcProtHead* &recvHead, string &recvstring)
{
    int ret = 0;
    int thislen = 0;
    int leftlen = 0;

    if(!recvstring.size()){
        return 0;
    }

    recvHead = new RpcProtHead;
    if((int)recvstring.size() < recvHead->HeadSize()){
        LOGD("%s msg not complete X1 %zu\n", __FUNCTION__, recvstring.size());
        goto FAIL;
    }

    if(recvstring.c_str()[0] == (char)0xef){
        if(!processHeartBeatMsgData(recvstring)){
            recvstring = recvstring.substr(8);
        }
        goto FAIL;
    }

    recvHead->initWithRecv(recvstring);
    thislen = recvHead->HeadSize() + recvHead->mHead.datalen;
    leftlen = recvstring.size() - thislen;

    if(leftlen < 0){
        LOGD("%s msg not complete X2\n", __FUNCTION__);
        ret = -1;
        goto FAIL;
    }


    if (recvHead->hasExtraData()) {
        LOGD("TODO ============has extra DATA\n");
        //recvHead->extradata = ;
    }

    if ((EMsgCmdType_Notify == recvHead->mHead.cmdtype) || (EMsgCmdType_Req == recvHead->mHead.cmdtype)){
        /*
        data{
            4B  method length ml;uint32_t
            mlB method;
            4B  subDataSize sds;uint32_t
            sdsB subData
        }
        */
#if 0
        int tempLen = Net2HostLong(*(uint32_t *)(recvstring.c_str() + recvHead->Size()));

        recvnode.method = recvstring.substr(recvHead->Size() + 4, tempLen);
        tempLen = Net2HostLong(*(uint32_t *)(recvstring.c_str() + recvHead->Size() + 4 + tempLen));

        recvnode.data = string(recvstring.c_str() + recvHead->Size() + 4 + recvnode.method.size() + 4, tempLen);
#endif
    }else{
        recvHead->data.assign(recvstring.c_str() + recvHead->HeadSize(), recvHead->mHead.datalen);
    }

    if(recvHead->isEncrpted()){
#if 0
        EncryptUtils eu;
        //PrintBuffer(recvnode.data.c_str(), recvnode.data.size());
        recvHead->data = eu.DecodeAES(getAesKey(), recvnode.data);
        //LOGD("dAES\n");
        //PrintBuffer(recvnode.data.c_str(), recvnode.data.size());
#endif
    }
    if(recvHead->isCompressed()){
        uint8_t bufout[20480] = "\0";
        size_t len = 0;
        qlz_state_decompress state;
        len = qlz_size_decompressed(recvHead->data.c_str());
        len = qlz_decompress(recvHead->data.c_str(), bufout, &state);
        recvHead->data = string((const char *)bufout, len);
        //LOGD("dcomp\n");
        //PrintBuffer(recvnode.data.c_str(), recvnode.data.size());
    }
    recvstring = leftlen > 0 ? recvstring.substr(thislen, leftlen):"";
    return 1;

FAIL:
    if(recvHead){
        delete recvHead;
        recvHead = NULL;
    }
    return ret;
}


int TcpClient::dispatchMsg(string &recvdata)
{
    LOGD("%s len=%u E\n", __FUNCTION__, recvdata.size());
    static string g_bigfragbuf;
    int ret = 0;

    RpcProtHead *respmsg = NULL;
    while((ret = recvDataPreProcess(respmsg, recvdata)) == 1){
    //1.pre process get cmdtype and seq;
    //if cmdtype
        LOGI("dispatch cmdtype=%hhu seq=%hu\n", respmsg->mHead.cmdtype, respmsg->mHead.seq);

        switch(respmsg->mHead.cmdtype){
            case EMsgCmdType_Req:{
                break;
            }
            case EMsgCmdType_Rsp:{
                ThreadWrapper t(TcpClient::dealResponse, 2, respmsg, this);
                t.detach();
                respmsg = NULL;//will free in dealResponse thread, just set null
                break;
            }
            case EMsgCmdType_Notify:{
                //dealNotify(respmsg);
                break;
            }
            case ERPCMSGTYPE_SYSRSP:{
                //PrintBuffer(respmsg->data.c_str(), respmsg->data.length());
                LOGD("ERPCMSGTYPE_SYSRSP not done====\n");
                break;
            }
            case ERPCMSGTYPE_DATAFRAG:{
                g_bigfragbuf.append(respmsg->data);
                ret = dispatchMsg(g_bigfragbuf);
                if(!ret){
                    LOGD("ERPCMSGTYPE_DATAFRAG done====%d \n", ret);
                    g_bigfragbuf.clear();
                }
                break;
            }
            case (uint8_t)EMsgCmdType_ConnLost:{
                LOGD("EMsgCmdType_ConnLost not done====\n");
                break;
            }
            default:{
                LOGD("unhandled message type %d");
                break;
            }
        }
        if(respmsg){
            delete respmsg;
        }
    }

    //delete msgNodeP;
    return ret;
}


int TcpClient::close(bool detach)
{
    if(!mClose){
        mClose = true;
        ictnet_close(mSocket, 0);
        ictnet_release();
    }
    mSocket = -1;
    mState = INIT;
    return 0;
}

int TcpClient::insertWaitNode(MsgStub *msg)
{
    mRespMutexCond.lock();
    LOGD("waitnodemap size = %d\n", mRespMap.size());
    while(mRespMap.size() > MAX_WAIT_QUEUE_SIZE){
        for(auto i = mRespMap.begin(); i != mRespMap.end();){
            if(i->second->getWaitTimeSeconds() > WAIT_TIMEOUT/1000){
                delete i->second;
                i=mRespMap.erase(i);
            }else{
                ++i;
            }
        }
        if(mRespMap.size() > MAX_WAIT_QUEUE_SIZE){
            if(!mRespMutexCond.wait(WAIT_TIMEOUT)){
                mRespMutexCond.unlock();
                return -1;
            }
        }else{
            break;
        }
    }
    mRespMap.insert(make_pair(msg->mHead.seq, msg));
    mRespMutexCond.unlock();
    return 0;
}


int TcpClient::dealResponse(int /*argc*/, void *argv[])
{
    bool isSync = false;
    map <uint16_t, MsgStub *>::iterator it;

    RpcProtHead *respmsg = (RpcProtHead *)argv[0];
    TcpClient *session = (TcpClient *)argv[1];
    MsgStub *waitNode = NULL;

    session->mRespMutexCond.lock();
    it = session->mRespMap.find(respmsg->mHead.seq);
    if(it != session->mRespMap.end()){
        waitNode = it->second;
        isSync = waitNode->IsSync();
        waitNode->dealResponse(respmsg);
        if(!isSync){
            delete waitNode;
        }
        session->mRespMap.erase(it);
        session->mRespMutexCond.signal();
        session->mRespMutexCond.unlock();
    }else{
        LOGD("no request match this response seqid=%hu\n", respmsg->mHead.seq);
    }
    delete respmsg;
    return 0;
}
