#ifndef __NEWPLAN_CLIENT_H__
#define __NEWPLAN_CLIENT_H__
#include <string>
#include <thread>
#include <functional>
#include "../utils/blockingQueue.h"

class TCPClient
{
public:
    TCPClient();
    ~TCPClient();

    void setup(std::string ip, short port);
    void start_service();
    void start_service2();

    void send_message(int sock_fd, BlockingQueue<int> *queue);
    int recv_message(int sock_fd);

    void send_data(int data_size);

    BlockingQueue<int> *get_send_channel();

private:
    std::string ip_addr;
    short port;
    int sock;
    std::shared_ptr<std::thread> background_thread;
    std::function<void(void)> callbacks;

    std::shared_ptr<std::thread> send_thread;
    std::shared_ptr<std::thread> recv_thread;

    BlockingQueue<int> *send_channel = nullptr;
};

#endif