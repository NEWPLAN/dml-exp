#ifndef __NEWPLAN_CLIENT_H__
#define __NEWPLAN_CLIENT_H__
#include <string>
class TCPClient
{
public:
    TCPClient();
    ~TCPClient();

    void connect_to_server(std::string ip, short port);
    void start_service();

private:
    std::string ip_addr;
    short port;
    int sock;
};

#endif