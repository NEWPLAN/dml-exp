#ifndef __NEWPLAN_TOWER_H__
#define __NEWPLAN_TOWER_H__

#include "tcp/client.h"
#include "tcp/server.h"
#include "../utils/blockingQueue.h"
#include <vector>
// #include "rdma/RDMAServer.h"
// #include "rdma/RDMAClient.h"

typedef enum
{
    UPPER_STREAM = 2,
    DOWN_STREAM
} TYPE;

class Tower
{
public:
    Tower(TYPE type);
    ~Tower();
    void send_message(int size);
    int receive_message(int size);
    void build_network_system(std::string ip, int port);
    void build_network_system(std::vector<std::string> ip, int port);
    void start_service();

    void register_signal_event(BlockingQueue<int> *signal_queue)
    {
        this->event_signal = signal_queue;
        server_guard->register_signal_event(event_signal, type_ == UPPER_STREAM ? 3 : 2);
    }

    void setup_chunk_size(int chunk_size)
    {
        this->chunk_size = chunk_size;
        server_guard->setup_chunk_size(chunk_size);
    }

private:
    BlockingQueue<int> *send_queue = nullptr;
    BlockingQueue<int> *recv_queue = nullptr;

    TCPServer *server_guard;
    std::vector<TCPClient *> client_group;
    std::vector<TCPClient *> server_group;
    TYPE type_;
    int listen_port;
    std::string server_ip;
    std::vector<std::string> ip_groups;
    BlockingQueue<int> *event_signal;
    int chunk_size;
};
#endif