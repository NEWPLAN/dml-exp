#include "msgstub.h"

#define LOG_TAG "msgstub"
#include "publicfun/debug.h"


MsgStub::MsgStub():
     mStatus(RESP_INIT),mContMutex(NULL),mAddTime(time(NULL))
{
}


MsgStub::~MsgStub()
{
    if(mContMutex){
        delete mContMutex;
        mContMutex = NULL;
    }
}

int MsgStub::waitDone()
{
    if(mContMutex){
        mContMutex->lock();
        if(mStatus == RESP_INIT){
            LOGD("wait");
            bool ret = mContMutex->wait(WAIT_TIMEOUT);
            if(mStatus == RESP_RECVED){
                mContMutex->wait();
            }
        }
        LOGD("wait Done X");
        mContMutex->unlock();
    }

    return mStatus == RESP_DEALDONE;
}

void MsgStub::done()
{
    if(mContMutex){
        mContMutex->lock();
        mStatus = RESP_DEALDONE;
        mContMutex->signal();
        mContMutex->unlock();
    }

}

void MsgStub::received()
{
    if(mContMutex){
        mContMutex->lock();
        mStatus = RESP_RECVED;
        mContMutex->signal();
        mContMutex->unlock();
    }
}

int MsgStub::dealResponse(RpcProtHead* msg)
{
    received();
    int ret = _dealResponse(msg);
    done();
    return ret;
}


void MsgStub::SetSync() {
    mContMutex = new CondMutex;
}


bool MsgStub::IsSync() {
    return mContMutex != NULL;
}


int MsgStub::makeRequest(uint16_t seq) {
    mHead.seq = seq;
    return _makeRequest();
}

