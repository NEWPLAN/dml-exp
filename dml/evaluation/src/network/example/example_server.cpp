#include "server.h"
#include <stdio.h>
#include <iostream>

static void userHelp(const char *str)
{
    printf("%s [local_ip] [local_port]\n", str);
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        userHelp(argv[0]);
        return 1;
    }
    TCPServer server;
    server.setup(argv[1], atoi(argv[2]));
    server.start_service();
    do
    {
        std::cout << "sleep for main thread" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(2));
    } while (1);
    return 0;
}