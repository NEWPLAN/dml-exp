#ifndef __NEWPLAN_SERVER_H__
#define __NEWPLAN_SERVER_H__
#include <string>
#include <thread>
#include <vector>
#include <functional>

class TCPServer
{
public:
    TCPServer();
    ~TCPServer();

    void setup(const std::string ip, short port);
    void handle_request(int fd);
    void start_service(int service_number = 5);

private:
    std::string ip_address;
    short port;
    int server_socket;
    std::thread *connector_thread = nullptr;
    std::vector<std::thread *> worker_threads;

    std::function<void(void)> callbacks;
};

#endif