#include "tcpserver.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define LOG_TAG "tcpserver"
#include "publicfun/debug.h"

#define ASSERT(expr)                                           \
    do                                                         \
    {                                                          \
        if (!(expr))                                           \
        {                                                      \
            fprintf(stderr,                                    \
                    "Assertion failed in %s on line %d: %s\n", \
                    __FILE__,                                  \
                    __LINE__,                                  \
                    #expr);                                    \
            abort();                                           \
        }                                                      \
    } while (0)

void FreeSendBuf(MySendBuf *buf)
{
    for (int i = 0; i < buf->bufcount; i++)
    {
        free(buf[i].buf);
    }
}

void write_done(uv_write_t *req, int status)
{
    LOGI("%s %p %pE", __func__, req, req->data);
    FreeSendBuf((MySendBuf *)req->data);
    free(req);

    if (status == 0)
    {
        LOGI("%s", __func__);
    }
    else
    {

        LOGE("uv_write error: %s - %s\n",
             uv_err_name(status),
             uv_strerror(status));
    }
}
static int fillbuf(const string &data, uv_buf_t &buf)
{
    buf.base = (char *)malloc(data.size());
    buf.len = data.size();
    memcpy(buf.base, data.c_str(), data.size());
    return 0;
}

static void alloc_cb(uv_handle_t * /*handle*/, size_t suggested_size, uv_buf_t *buf)
{
    buf->base = (char *)malloc(suggested_size);
    buf->len = suggested_size;
}

static void on_close(uv_handle_t *peer)
{
    free(peer);
}

static void after_shutdown(uv_shutdown_t *req, int status)
{
    uv_close((uv_handle_t *)req->handle, on_close);
    free(req);
}

std::string GetUVError(int retcode)
{
    std::string err;
    err = uv_err_name(retcode);
    err += ":";
    err += uv_strerror(retcode);
    return err;
}

//////////////////////// START ///////////////////////////

TcpServer::TcpServer() : mInited(false)
{
    mLoop = uv_default_loop();
    mTcpServer.data = this;
}

TcpServer::~TcpServer()
{
}

int TcpServer::Start(const char *ip, const uint16_t port)
{
    if (mInited)
    {
        return 0;
    }
    struct sockaddr_in addr;
    if (0 != uv_ip4_addr(ip, port, &addr))
    {
        return -1;
    }

    //init server

    return initServer((const struct sockaddr *)&addr);
}

int TcpServer::Start6(const char *ip, const uint16_t port)
{
    if (mInited)
    {
        return 0;
    }
    struct sockaddr_in6 addr;
    int iret = uv_ip6_addr(ip, port, &addr);
    if (iret)
    {
        return -1;
    }
    return initServer((const struct sockaddr *)&addr);
}

int TcpServer::initServer(const struct sockaddr *addr)
{
    int r = 0;
    r = uv_mutex_init(&mutex_handle_);

    r = uv_tcp_init(mLoop, &mTcpServer);
    if (r)
    {
        /* TODO: Error codes */
        fprintf(stderr, "Socket creation error\n");
        return 1;
    }
    r = uv_tcp_bind(&mTcpServer, addr, 0);
    if (r)
    {
        /* TODO: Error codes */
        fprintf(stderr, "Bind error\n");
        return 1;
    }

    r = uv_listen((uv_stream_t *)&mTcpServer, SOMAXCONN, on_new_connection);
    if (r)
    {
        /* TODO: Error codes */
        fprintf(stderr, "Listen error %s\n", uv_err_name(r));
        return 1;
    }

    printf("server runing.\n");
    int iret = uv_run(mLoop, UV_RUN_DEFAULT);
    if (iret)
    {
        string errmsg_ = GetUVError(iret);
        LOGE("%s", errmsg_.c_str());
        return -1;
    }
    mInited = true;
    return r;
}

void TcpServer::on_new_connection(uv_stream_t *server, int status)
{
    TcpServer *that = (TcpServer *)server->data;
    uv_stream_t *aclient;
    int r;

    if (status != 0)
    {
        LOGE("%s:%s\n", __func__, GetUVError(status).c_str());
        return;
    }

    aclient = (uv_stream_t *)malloc(sizeof(uv_tcp_t));
    ASSERT(aclient != NULL);
    r = uv_tcp_init(that->mLoop, (uv_tcp_t *)aclient);
    ASSERT(r == 0);

    r = uv_accept(server, aclient);
    ASSERT(r == 0);

    r = that->AddClient(aclient);

    r = uv_read_start(aclient, alloc_cb, read_cb);
    ASSERT(r == 0);
}

int TcpServer::DeleteClient(uv_stream_t *client)
{
    uv_shutdown_t *sreq;
    sreq = (uv_shutdown_t *)malloc(sizeof *sreq);
    ASSERT(0 == uv_shutdown(sreq, client, after_shutdown));

    LOGI("%s:mClientsMap size = %lu, erase %p", __func__, mClientsMap.size(), client);
    uv_mutex_lock(&mutex_handle_);
    map<uv_stream_t *, ClientInfo *>::iterator item = mClientsMap.find(client);
    if (item != mClientsMap.end())
    {
        delete (item->second);
        mClientsMap.erase(item);
    }
    uv_mutex_unlock(&mutex_handle_);

    LOGI("mClientsMap size = %lu", mClientsMap.size());
    return 0;
}

int TcpServer::AddClient(uv_stream_t *client)
{
    ClientInfo *data = newClient();
    data->client = client;
    data->tcpserver = this;
    client->data = data;

    uv_mutex_lock(&mutex_handle_);
    mClientsMap[client] = data;
    uv_mutex_unlock(&mutex_handle_);
    return 0;
}

struct DataDealRequestData
{
    TcpServer *server;
    uv_stream_t *client;
};

void TcpServer::read_cb(uv_stream_t *handle, ssize_t nread, const uv_buf_t *buf)
{
    ClientInfo *aclient = (ClientInfo *)handle->data;
    if (nread < 0)
    {

        if (nread == UV_EOF)
        {
            LOGI("客户端(%p)连接断开，关闭此客户端", handle);
        }
        else if (nread == UV_ECONNRESET)
        {
            LOGI("客户端(%p)异常断开", handle);
        }
        else
        {
            LOGI("%s\n", GetUVError(nread).c_str());
        }

        free(buf->base);
        aclient->tcpserver->DeleteClient(handle); //连接断开，关闭客户端
        return;
    }

    if (nread == 0)
    {
        /* Everything OK, but nothing read. */
        LOGI("Everything OK, but nothing read.\n");
        free(buf->base);
        return;
    }

    LOGV("%s %p %zu", __func__, handle, nread);
    aclient->lockrecvdata();
    aclient->mRecvDataCache.append(buf->base, nread);
    aclient->unlockrecvdata();

    uv_work_t *req = (uv_work_t *)malloc(sizeof(uv_work_t));
    DataDealRequestData *reqdata = (DataDealRequestData *)malloc(sizeof(DataDealRequestData));
    reqdata->server = aclient->tcpserver;
    reqdata->client = handle;
    req->data = reqdata;
    uv_queue_work(aclient->tcpserver->mLoop, req, callback, callbackDone);

    free(buf->base);
}

void TcpServer::callback(uv_work_t *req)
{
    LOGI("%s E", __func__);
    DataDealRequestData *reqdata = (DataDealRequestData *)req->data;
    //LOGBIN(cdata->mRecvDataCache.c_str(), cdata->mRecvDataCache.size());

    //TODO deal data
    uv_mutex_lock(&(reqdata->server->mutex_handle_));
    map<uv_stream_t *, ClientInfo *>::iterator item = reqdata->server->mClientsMap.find(reqdata->client);
    if (item != reqdata->server->mClientsMap.end())
    {
        LOGV("%s:found client %p", __func__, reqdata->client);
        ClientInfo *cdata = item->second;
        string resp;
        cdata->lockrecvdata();
        cdata->dealClientData(cdata->mRecvDataCache, resp);
        cdata->unlockrecvdata();

        if (!resp.empty())
        {
            cdata->locksendbuf();
            cdata->mSendbuf.buf = (uv_buf_t *)realloc(cdata->mSendbuf.buf, cdata->mSendbuf.bufcount + 1 * sizeof(cdata->mSendbuf.buf));
            fillbuf(resp, cdata->mSendbuf.buf[cdata->mSendbuf.bufcount++]);
            cdata->unlocksendbuf();
            resp.clear();
        }
    }
    uv_mutex_unlock(&(reqdata->server->mutex_handle_));
    LOGI("%s X", __func__);
}

void TcpServer::callbackDone(uv_work_t *req, int)
{
    LOGI("%s E", __func__);
    DataDealRequestData *reqdata = (DataDealRequestData *)req->data;

    uv_mutex_lock(&reqdata->server->mutex_handle_);
    map<uv_stream_t *, ClientInfo *>::iterator item = reqdata->server->mClientsMap.find(reqdata->client);
    if (item != reqdata->server->mClientsMap.end())
    {
        LOGV("%s:found client %p", __func__, reqdata->client);
        ClientInfo *cdata = item->second;
        if (cdata->mSendbuf.bufcount > 0)
        {
            uv_write_t *wr;
            wr = (uv_write_t *)malloc(sizeof *wr);
            ASSERT(wr != NULL);

            cdata->locksendbuf();
            wr->data = &cdata->mSendbuf;
            LOGI("%s:buf=%p count=%d", __func__, cdata->mSendbuf.buf, cdata->mSendbuf.bufcount);
            if (uv_write(wr, cdata->client, cdata->mSendbuf.buf, cdata->mSendbuf.bufcount, write_done))
            {
                LOGE("uv_write failed");
            }
            cdata->mSendbuf.buf = NULL;
            cdata->mSendbuf.bufcount = 0;
            cdata->unlocksendbuf();
        }
        else
        {
            LOGV("%s:resp empty", __func__);
        }
    }
    uv_mutex_unlock(&reqdata->server->mutex_handle_);

    free(req);
    LOGI("%s X", __func__);
}

void TcpServer::close()
{
    uv_mutex_lock(&mutex_handle_);
    for (map<uv_stream_t *, ClientInfo *>::iterator it = mClientsMap.begin(); it != mClientsMap.end(); ++it)
    {
        delete (it->second);
        uv_close((uv_handle_t *)it->first, on_close);
    }
    mClientsMap.clear();
    uv_mutex_unlock(&mutex_handle_);

    LOGI("close server");
    if (mInited)
    {
        uv_close((uv_handle_t *)&mTcpServer, NULL);
        LOGI("close server");
    }
    mInited = false;

    uv_mutex_destroy(&mutex_handle_);
}
