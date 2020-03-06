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
#include <thread>
#include <chrono>

TCPClient::TCPClient()
{
    this->callbacks = []() { std::cout << "Default client callbacks after send" << std::endl; };
    sock = ::socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        std::cout << "Creating socket error" << std::endl;
        exit(-1);
    }
    std::cout << "Client has been created" << std::endl;
}
TCPClient::~TCPClient()
{
    ::close(sock);
    std::cout << "On closing socket" << std::endl;
}

void TCPClient::setup(std::string ip, short port = 12345)
{
    this->ip_addr = ip;
    this->port = port;
    struct sockaddr_in client;
    client.sin_family = AF_INET;
    client.sin_port = ::htons(port);
    client.sin_addr.s_addr = ::inet_addr(ip.c_str());

    {
        int time_out = 0;
        while (connect(sock, (struct sockaddr *)&client, sizeof(client)) != 0)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            time_out += 1;
            if (time_out % 20 == 0)
            {
                std::cout << "[" << time_out * 50 / 1000 << "] Checking your server has been opened..." << std::endl;
            }
            if (time_out > 200)
            {
                std::cerr << "On connection error in " << time_out / 1000 << " s" << std::endl;
                close(sock);
                exit(-2);
            }
        }
    }
    std::cout << "Client has connected to " << ip << ":" << port << std::endl;
}
void TCPClient::start_service()
{
    this->background_thread = std::make_shared<std::thread>(std::thread([this]() {
        size_t index = 0;
        struct timeval start, stop;
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
                this->callbacks();
                gettimeofday(&stop, 0);
                long res = (stop.tv_sec - start.tv_sec) * 1000000 + stop.tv_usec - start.tv_usec;
                printf("index: %lu, rate: %10f Mb/s\n", index, 1.024 * 1.024 * 10000.0 / (res / 1000000.0));
                start = stop;
            }
        }
    }));
}
