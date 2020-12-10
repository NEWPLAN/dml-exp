#ifndef MSGSTUB_H
#define MSGSTUB_H

#include "publicfun/condmutex.h"
#include "rpchead.h"

#define WAIT_TIMEOUT 30000   //30 seconds

enum MsgStatus{
    RESP_INIT = 0,
    RESP_RECVED = 1,
    RESP_DEALDONE = 2
};

/**
 * @todo write docs
 */
class MsgStub:public RpcProtHead
{
public:
    /**
     * Default constructor
     */
    MsgStub();

    /**
     * Destructor
     */
    virtual ~MsgStub();

    int makeRequest(uint16_t seq);
    int dealResponse(RpcProtHead *msg);
    int waitDone();

    void SetSync();
    bool IsSync();

    time_t getWaitTimeSeconds(){
        return time(NULL) - mAddTime;
    }
protected:
    bool mIsSync;
    virtual int _makeRequest() = 0;
    virtual int _dealResponse(RpcProtHead *) = 0;
    MsgStatus mStatus;
    CondMutex *mContMutex;
    time_t mAddTime;
    void done();
    void received();
};

#endif // MSGSTUB_H
