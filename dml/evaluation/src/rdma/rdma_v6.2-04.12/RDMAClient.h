#ifndef __RDMA_CLIENT_H__
#define __RDMA_CLIENT_H__
#include <string>
#include "RDMABase.h"
#include <thread>

class RDMAClient : public RDMABase
{
public:
    RDMAClient(RDMAAdapter &rdma_adapter);
    ~RDMAClient();

    void setup();
    void start_service();

protected:
    virtual void *poll_cq(void *_id);
    virtual void on_connection(struct rdma_cm_id *id) {}
    virtual void on_pre_conn(struct rdma_cm_id *id);
    void on_completion(struct ibv_wc *wc);
    void on_disconnect(struct rdma_cm_id *id);
    void event_loop(struct rdma_event_channel *ec);

private:
    void post_receive(uint32_t msg_id);
    void write_remote(uint32_t buffer_id, uint32_t len);
    void send_next_chunk(uint32_t buffer_id);
    void send_file_name(struct rdma_cm_id *id);

private:
    void _send_loops();
    void _init();

private:
    int port_ = 2020;
    std::string ip_addr_;

    std::thread *send_thread = nullptr;
    RDMAAdapter rdma_adapter_;
    struct sockaddr_in ser_in, local_in; /*server ip and local ip*/

    struct RDMAContext *ctx = 0;
};

#endif