#ifndef __NEWPLAN_SERVER_H__
#define __NEWPLAN_SERVER_H__
#include <string>
#include <thread>
#include <vector>
#include <functional>
#include "../utils/blockingQueue.h"

class TCPServer
{
public:
    TCPServer();
    ~TCPServer();

    void setup(const std::string ip, short port);
    void start_service(int service_number = 5);

    void start_service2();

    BlockingQueue<int> *get_recv_channel();

    void send_message(int sock_fd);
    int recv_message(int sock_fd, BlockingQueue<int> *queue);
    int recv_message_fix_size(int sock_fd, BlockingQueue<int> *queue);

    void register_signal_event(BlockingQueue<int> *signal_queue, int signal)
    {
        this->event_signal = signal_queue;
        this->signal_ = signal;
    }

    void setup_chunk_size(int chunk_size) { this->chunk_size = chunk_size; }

private:
    std::string ip_address;
    short port;
    int server_socket;
    std::thread *connector_thread = nullptr;
    std::vector<std::thread *> worker_threads;
    BlockingQueue<int> *recv_channel = nullptr;

    std::function<void(void)> callbacks;

    BlockingQueue<int> *event_signal;
    int signal_ = 0;

    int chunk_size = 1000000;
};

#endif