#include "rpctcpserver.h"

#include "publicfun/quicklz/quicklz.h"
#include "encyptwrapper/crc32.h"
#include <unistd.h>

#define LOG_TAG "tcpserver"
#include "publicfun/debug.h"


int RpcClientProxy::processHeartBeatMsgData(string&)
{
    return 0;
}


int RpcClientProxy::recvDataPreProcess(RpcProtHead*& recvHead, string& recvstring)
{
    int ret = 0;
    int thislen = 0;
    int leftlen = 0;

    if(!recvstring.size()) {
        return 0;
    }

    recvHead = new RpcProtHead;
    if((int)recvstring.size() < recvHead->HeadSize()) {
        LOGD("%s msg not complete X1 %zu\n", __FUNCTION__, recvstring.size());
        goto FAIL;
    }

    if(recvstring.c_str()[0] == (char)0xef) {
        if(!processHeartBeatMsgData(recvstring)) {
            recvstring = recvstring.substr(8);
        }
        goto FAIL;
    }

    recvHead->initWithRecv(recvstring);
    thislen = recvHead->HeadSize() + recvHead->mHead.datalen;
    leftlen = recvstring.size() - thislen;

    if(leftlen < 0) {
        LOGD("%s msg not complete X2\n", __FUNCTION__);
        ret = -1;
        goto FAIL;
    }


    if (recvHead->hasExtraData()) {
        LOGD("TODO ============has extra DATA\n");
        //recvHead->extradata = ;
    }

    if ((EMsgCmdType_Notify == recvHead->mHead.cmdtype) || (EMsgCmdType_Req == recvHead->mHead.cmdtype)) {
        recvHead->data.assign(recvstring.c_str() + recvHead->HeadSize(), recvHead->mHead.datalen);
    }

    if(recvHead->isEncrpted()) {
#if 0
        EncryptUtils eu;
        //PrintBuffer(recvnode.data.c_str(), recvnode.data.size());
        recvHead->data = eu.DecodeAES(getAesKey(), recvnode.data);
        //LOGD("dAES\n");
        //PrintBuffer(recvnode.data.c_str(), recvnode.data.size());
#endif
    }
    if(recvHead->isCompressed()) {
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
    if(recvHead) {
        delete recvHead;
        recvHead = NULL;
    }
    return ret;
}


int RpcClientProxy::ProcessMsgBeforeSend(RpcProtHead* rpcjmsg, std::string& resp)
{
    int ret = 0;

    char localPackData[16] = "\0";

    CRC32 crc32;
    rpcjmsg->mHead.datacrc = crc32.crc32(rpcjmsg->data.c_str(), rpcjmsg->data.size());
    rpcjmsg->mHead.datalen = rpcjmsg->data.size();

    rpcjmsg->packHead(localPackData);
    LOGI("postMsg seq=%u\n", rpcjmsg->mHead.seq);
    resp.append(localPackData, 16);
    resp.append(rpcjmsg->data);
    LOGV("%s x with %d\n", __FUNCTION__, ret);

    return ret;
}


int RpcClientProxy::dealClientData(std::__cxx11::string& recvdata, std::__cxx11::string& serverresp) {
    LOGD("%s len=%u E\n", __FUNCTION__, recvdata.size());
    static string g_bigfragbuf;
    int ret = 0;

    RpcProtHead *protodata = NULL;
    while((ret = recvDataPreProcess(protodata, recvdata)) == 1) {
        //1.pre process get cmdtype and seq;
        //if cmdtype
        LOGI("dispatch cmdtype=%hhu seq=%hu\n", protodata->mHead.cmdtype, protodata->mHead.seq);

        switch(protodata->mHead.cmdtype) {
        case EMsgCmdType_Req: {
            LOGI("%s:%s %zu", __func__, protodata->data.c_str(), protodata->data.size());

            ret = dealreq(protodata);
            ProcessMsgBeforeSend(protodata, serverresp);

            break;
        }
        case EMsgCmdType_Rsp: {

            break;
        }
        case EMsgCmdType_Notify: {
            //dealNotify(respmsg);
            break;
        }
        case ERPCMSGTYPE_SYSRSP: {
            //PrintBuffer(respmsg->data.c_str(), respmsg->data.length());
            LOGD("ERPCMSGTYPE_SYSRSP not done====\n");
            break;
        }
        case ERPCMSGTYPE_DATAFRAG: {
            g_bigfragbuf.append(protodata->data);
            ret = dealClientData(g_bigfragbuf, serverresp);
            if(!ret) {
                LOGD("ERPCMSGTYPE_DATAFRAG done====%d \n", ret);
                g_bigfragbuf.clear();
            }
            break;
        }
        case (uint8_t)EMsgCmdType_ConnLost: {
            LOGD("EMsgCmdType_ConnLost not done====\n");
            break;
        }
        default: {
            LOGD("unhandled message type %d");
            break;
        }
        }
        if(protodata) {
            delete protodata;
        }
    }

    //delete msgNodeP;
    return ret;
}

