#ifndef __NEWPLAN_CLIENT_H__
#define __NEWPLAN_CLIENT_H__
#include <string>
#include <thread>
#include <functional>
class TCPClient
{
public:
    TCPClient();
    ~TCPClient();

    void setup(std::string ip, short port);
    void start_service();

private:
    std::string ip_addr;
    short port;
    int sock;
    std::shared_ptr<std::thread> background_thread;
    std::function<void(void)> callbacks;
};

#endif