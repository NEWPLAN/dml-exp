#ifndef __RDMA_BASE_H__
#define __RDMA_BASE_H__

#include <vector>
#include <string>
#include <pthread.h>

#include <rdma/rdma_cma.h>

#define BUFFER_SIZE (16 * 1 * 1024)
#define WINDOWS_NUM 256
#define MAX_DATA_IN_FLIGHT 128

//#define WINDOW_SIZE (BUFFER_SIZE * MAX_DATA_IN_FLIGHT)
//****************************************************************
// |0|1|2|...|MAX_DATA-1|.................|0|1|2|...|MAX_DATA-1|
// |---- WIN_ID = 0 ----|-- WIN_ID = XX --|-- WIN_ID = 255 ----|
//****************************************************************

#define BATCH_MSG 48
#define MSG_NUM_OFFEST (BATCH_MSG + 1)
#define WINDOW_ID_OFFEST BATCH_MSG

//******switch*************
//#define DEBUG_REVERSE_ORDER

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
        //short batch_index[4];
        uint8_t batch_index[BATCH_MSG + 2];
    } data;
};

struct ConnectionInfo
{
    char local_addr[48];
    char peer_addr[48];
    short local_port;
    short peer_port;
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
    struct message *msg = 0;
    struct ibv_mr *msg_mr = 0;

    //store peer information
    uint64_t peer_addr = 0;
    uint32_t peer_rkey = 0;

    // connection management handle
    struct rdma_cm_id *id = 0;
    size_t recv_bytes = 0;
    char connection_id[20] = {0};
    int client_index = -1;

    uint32_t window_id = 0;

    struct ConnectionInfo ctx_info;

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

    void set_server_port(uint32_t port)
    {
        this->server_port = port;
    }
    std::string get_client_ip() { return client_ip; }
    std::string get_server_ip() { return server_ip; }
    uint32_t get_server_port() { return server_port; }
};

#include <unistd.h>
#include <sys/syscall.h>
#include <iostream>
inline uint32_t get_tid()
{
#if defined(APPLE)
    return static_cast<std::uint32_t>(std::this_thread::get_id());
#else
    return static_cast<std::uint32_t>(syscall(SYS_gettid));
#endif
}

#define LOG_INFO(format, ...)            \
    do                                   \
    {                                    \
        printf("[%u] [%s:%d] ",          \
               get_tid(),                \
               __FILE__,                 \
               __LINE__);                \
        log_info(format, ##__VA_ARGS__); \
    } while (0)

#define TIMEOUT_IN_MS 500
#define TEST_NZ(x)                                               \
    do                                                           \
    {                                                            \
        if ((x))                                                 \
        {                                                        \
            printf("[%s:%d]: ", __FILE__, __LINE__);             \
            rc_die("error: " #x " failed (returned non-zero)."); \
        }                                                        \
    } while (0)
#define TEST_Z(x)                                                 \
    do                                                            \
    {                                                             \
        if (!(x))                                                 \
        {                                                         \
            printf("[%s:%d]: ", __FILE__, __LINE__);              \
            rc_die("error: " #x " failed (returned zero/null)."); \
        }                                                         \
    } while (0)

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
    virtual void on_disconnect(struct rdma_cm_id *id) = 0;
    virtual void on_connection(struct rdma_cm_id *id) = 0;
    virtual void on_pre_conn(struct rdma_cm_id *id) = 0;

    virtual void build_connection(struct rdma_cm_id *id);
    virtual void build_qp_attr(struct rdma_cm_id *id,
                               struct ibv_qp_init_attr *qp_attr);
    virtual void build_context(struct rdma_cm_id *id);
    virtual void build_params(struct rdma_conn_param *params);

    virtual struct RDMAContext *get_or_build_ctx(struct rdma_cm_id *id);

    virtual void rc_die(const char *reason);
    virtual void log_info(const char *format, ...);

public:
    virtual struct ibv_pd *rc_get_pd(struct rdma_cm_id *id);

private:
    RDMAAdapter *rdma_adapter_;
    size_t recv_count_ = 0;
};

#endif