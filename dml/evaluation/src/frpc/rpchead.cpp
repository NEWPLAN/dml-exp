#include "rpchead.h"
#include <string.h>

#define LOG_TAG "rpchead"
#include "publicfun/debug.h"

#ifndef LOGD
#define LOGD(a, ...) printf(a, __VA_ARGS__)
#endif


RpcProtHead::RpcProtHead()
{
    memset(&mHead, 0, sizeof(mHead));
    mHead.magic = FRPC_MAGIC;
    mHead.version = 1;
}

void RpcProtHead::initWithRecv(string &parm)
{
    if(parm.length() == 0){
        LOGD("%s length = 0 X", __FUNCTION__);
        return;
    }
    mHead = *(protoHead *)parm.c_str();
    headdump();
}

void RpcProtHead::headdump()
{
    LOGD("magic:0x%02x;version:0x%02x;cmdtype:0x%02x;flag:0x%02x;\nheadcrc:0x%04x;datacrc:0x%04x;\n\
seq:%d;timeout:%d;datalen=%d\n", mHead.magic, mHead.version, mHead.cmdtype,mHead.flag,
            mHead.headcrc16,
            mHead.datacrc,
            mHead.seq,
            mHead.timeout,
            mHead.datalen);
}

void RpcProtHead::setEncrpted(bool paramBoolean)
{
    if (paramBoolean){
        mHead.flag |= 0x02;
    }else{
        mHead.flag &= 0xFD;
    }
}

void RpcProtHead::setCompressed(bool paramBoolean)
{
    if (paramBoolean){
        mHead.flag |= 0x1;
    }else{
        mHead.flag &= 0xFE;
    }
}

void RpcProtHead::setExtradata(string &paramArrayOfByte)
{
    this->extradata = paramArrayOfByte;
    if (paramArrayOfByte.length() == 0){
        mHead.flag &= 0xFB;
    }else{
        mHead.flag |= 0x4;
    }

}


void RpcProtHead::setBreadNetWidth(bool paramBoolean)
{
    if (paramBoolean) {
        mHead.flag |= 0x8;
    }
}

void RpcProtHead::packHead(char *paramPackData)
{
  mHead.headcrc16 = (0UL+(mHead.magic) +
                      (mHead.version) +
                      (mHead.cmdtype) +
                      (mHead.flag) +
                      (mHead.datacrc) +
                      (mHead.seq) +
                      (mHead.timeout) +
                      (mHead.datalen));

  memcpy(paramPackData, &mHead, sizeof(mHead));
}


/*
 * if ((localRpcProtHead.getFlag() & 0x4) != 0)
      {
        localObject1 = ((PackData)localObject2).unpackBytes();
        localRpcProtHead.setExtradata((byte[])localObject1);
        System.arraycopy(paramArrayOfByte, RpcProtHead.Size() + localObject1.length + 4, arrayOfByte, 0, localRpcProtHead.getDatalen() - (localObject1.length + 4));
        localObject1 = null;
      }

*/
bool RpcProtHead::hasExtraData()
{
    return (mHead.flag & 0x4) != 0;
}

bool RpcProtHead::isCompressed()
{
    return (mHead.flag & 0x1) != 0;
}

bool RpcProtHead::isEncrpted()
{
    return (mHead.flag & 0x2) != 0;
}
