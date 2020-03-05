/*************************************************************************
        > Copyright(c)  NEWPLAN, all rights reserved.
        > File Name   : client.c
        > Author      : NEWPLAN
        > Mail        : newplan001@163.com
        > Created Time: 2018年11月06日 星期二 16时22分01秒
 ************************************************************************/

#include "client.h"
#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>

TCPClient::TCPClient()
{
    sock = ::socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        std::cout << "Creating socket error" << std::endl;
        exit(-1);
    }
    std::cout << "Creating socket" << std::endl;
}
TCPClient::~TCPClient()
{
    ::close(sock);
    std::cout << "On closing socket" << std::endl;
}

void TCPClient::connect_to_server(std::string ip, short port = 12345)
{
    struct sockaddr_in client;
    client.sin_family = AF_INET;
    client.sin_port = ::htons(port);
    client.sin_addr.s_addr = ::inet_addr(ip.c_str());

    if (connect(sock, (struct sockaddr *)&client, sizeof(client)) < 0)
    {
        std::cerr << "On connection error" << std::endl;
        exit(-2);
    }
}
void TCPClient::start_service()
{
    size_t index = 0;
    struct timeval start, stop, diff;
    gettimeofday(&start, 0);
    while (1)
    {
        char buffer[1024 * 1024] = "hello world!";
        ssize_t s = 1024 * 1024;

        if (s > 0)
        {
            buffer[s - 1] = 0;
            write(sock, buffer, s);
            ssize_t _s = read(sock, buffer, s);
            if (_s > 0)
            {
                //printf("Received: %ld\n", atoi(buffer));
                buffer[_s] = 0;
                //printf("server echo# %s\n", buffer);
            }
        }
        if (index++ % 10000 == 0)
        {
            gettimeofday(&stop, 0);
            long res = (stop.tv_sec - start.tv_sec) * 1000000 + stop.tv_usec - start.tv_usec;
            printf("index: %lu, rate: %10f Mb/s\n", index, 1.024 * 1.024 * 10000.0 / (res / 1000000.0));
            start = stop;
        }
    }
}
