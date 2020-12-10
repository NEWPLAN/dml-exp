#ifndef TCPSERVER_H
#define TCPSERVER_H
#include "libuv/include/uv.h"
#include <map>
using namespace std;

class TcpServer;

struct MySendBuf{
    MySendBuf():buf(NULL), bufcount(0)
    {}
    uv_buf_t *buf;
    size_t bufcount;
};

struct ClientInfo{


    ClientInfo():tcpserver(NULL),client(NULL){

        uv_mutex_init(&mRecvLock);
        uv_mutex_init(&mSendBufLock);
    }

    virtual ~ClientInfo(){
        uv_mutex_destroy(&mRecvLock);
        uv_mutex_destroy(&mSendBufLock);
    }

    void lockrecvdata(){
        uv_mutex_lock(&mRecvLock);
    }
    void unlockrecvdata(){
        uv_mutex_unlock(&mRecvLock);
    }

    void locksendbuf(){
        uv_mutex_lock(&mSendBufLock);
    }
    void unlocksendbuf(){
        uv_mutex_unlock(&mSendBufLock);
    }

    virtual int dealClientData(string & cdata, string &resp) = 0;

    TcpServer *tcpserver;
    uv_stream_t* client;
    MySendBuf mSendbuf;
    string mRecvDataCache;
private:
    uv_mutex_t mRecvLock;//保护recvdata
    uv_mutex_t mSendBufLock;//保护buf

};

/**
 * @todo write docs
 */
class TcpServer
{
public:
    /**
     * Default constructor
     */
    TcpServer();

    /**
     * Destructor
     */
    virtual ~TcpServer();

    int Start(const char *ip, const uint16_t port);
    int Start6(const char *ip, const uint16_t port);

    void close();

protected:
    virtual ClientInfo* newClient() = 0;

private:

    int initServer(const struct sockaddr *addr);

    int DeleteClient(uv_stream_t*);
    int AddClient(uv_stream_t *);

    static void callback(uv_work_t *req);
    static void callbackDone(uv_work_t *req, int);

    static void on_new_connection(uv_stream_t *server, int status);
    static void read_cb(uv_stream_t* handle, ssize_t nread, const uv_buf_t* buf);


    uv_loop_t *mLoop;
    uv_tcp_t mTcpServer;

    map<uv_stream_t *, ClientInfo *> mClientsMap;
    uv_mutex_t mutex_handle_;//保护clients_list_
    bool mInited;
};

#endif // TCPSERVER_H
