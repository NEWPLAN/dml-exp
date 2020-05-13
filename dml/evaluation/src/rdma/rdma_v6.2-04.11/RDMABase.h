#ifndef __RDMA_BASE_H__
#define __RDMA_BASE_H__

#include <vector>
#include <string>
#include <pthread.h>

#include <rdma/rdma_cma.h>

#define BUFFER_SIZE (10 * 1 * 1024)
#define MAX_DATA_IN_FLIGHT 128

enum message_id
{
    MSG_INVALID = 0,
    MSG_MR,
    MSG_READY,
    MSG_DONE
};

struct message
{
    int id;
    union {
        struct
        {
            uint64_t addr;
            uint32_t rkey;
        } mr;
        short batch_index[4];
        //uint64_t buffer_map[4];
    } data;
};

struct RDMAContext
{
    struct ibv_context *ibv_ctx = 0;
    struct ibv_pd *pd = 0;
    struct ibv_cq *cq = 0;
    struct ibv_comp_channel *comp_channel = 0;

    //register buffer for remote to write
    uint8_t *buffer = 0;
    struct ibv_mr *buffer_mr = 0;

    //register message mem is used for command channel
    struct message *msg[MAX_DATA_IN_FLIGHT] = {0};
    struct ibv_mr *msg_mr[MAX_DATA_IN_FLIGHT] = {0};

    //store peer information
    uint64_t peer_addr = 0;
    uint32_t peer_rkey = 0;

    // connection management handle
    struct rdma_cm_id *id = 0;
    size_t recv_bytes = 0;
    char connection_id[20] = {0};
    int client_index = -1;

    struct timeval start;
};

#include <iostream>
struct RDMAAdapter
{
    unsigned short server_port;
    struct rdma_event_channel *event_channel;
    struct rdma_cm_id *listener;
    std::vector<rdma_cm_id *> recv_rdma_cm_id;

    std::string server_ip;
    std::string client_ip;
    RDMAAdapter()
    {
        server_port = 12345;
        server_ip = "";
        client_ip = "";
    }
    void set_server_ip(const char *_server_ip)
    {
        server_ip = _server_ip;
        std::cout << "Server IP : " << server_ip << std::endl;
    }
    void set_client_ip(const char *_client_ip)
    {
        client_ip = _client_ip;
        std::cout << "Client IP : " << client_ip << std::endl;
    }
    std::string get_client_ip() { return client_ip; }
    std::string get_server_ip() { return server_ip; }
};

#define LOG_INFO(format, ...)          \
    do                                 \
    {                                  \
        printf("[%s:%d] ",             \
               __FILE__,               \
               __LINE__);              \
        log_info(format, __VA_ARGS__); \
    } while (0)

#define TIMEOUT_IN_MS 500
#define TEST_NZ(x)                                               \
    do                                                           \
    {                                                            \
        if ((x))                                                 \
            rc_die("error: " #x " failed (returned non-zero)."); \
    } while (0)
#define TEST_Z(x)                                                 \
    do                                                            \
    {                                                             \
        if (!(x))                                                 \
            rc_die("error: " #x " failed (returned zero/null)."); \
    } while (0)

#define MIN_CQE 10

class RDMAConnector
{
public:
    RDMAConnector() {}
    ~RDMAConnector() {}

public:
    struct ibv_pd *get_protect_domain() { return this->pd; }
    struct ibv_cq *get_completion_queue() { return this->cq; }

private:
    struct ibv_context *ibv_ctx = 0;
    struct ibv_pd *pd = 0;
    struct ibv_cq *cq = 0;
    struct ibv_comp_channel *comp_channel = 0;

    //register buffer for remote to write
    char *buffer = 0;
    struct ibv_mr *buffer_mr = 0;
};

class RDMABase
{
public:
    RDMABase() {}
    virtual ~RDMABase() {}
    void show_performance(int time_duration = 1);
    void add_performance(size_t data_num);

protected:
    virtual long long current_time(void);

    virtual void on_disconnect(struct rdma_cm_id *id) = 0;
    virtual void on_connection(struct rdma_cm_id *id) = 0;
    virtual void on_pre_conn(struct rdma_cm_id *id) = 0;

    virtual void build_connection(struct rdma_cm_id *id);
    virtual void build_qp_attr(struct rdma_cm_id *id,
                               struct ibv_qp_init_attr *qp_attr);
    virtual void build_context(struct rdma_cm_id *id);
    virtual void build_params(struct rdma_conn_param *params);

    virtual void rc_die(const char *reason);
    virtual void log_info(const char *format, ...);

public:
    virtual struct ibv_pd *rc_get_pd(struct rdma_cm_id *id);
    //struct RDMAContext *s_ctx = 0;

private:
    RDMAAdapter *rdma_adapter_;
    size_t recv_count_ = 0;
};

#endif