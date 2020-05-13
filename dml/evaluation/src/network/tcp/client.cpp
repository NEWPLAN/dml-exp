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

#include "../utils/ATimer.h"

TCPClient::TCPClient()
{
    this->send_channel = new BlockingQueue<int>();
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

BlockingQueue<int> *TCPClient::get_send_channel()
{
    return this->send_channel;
}

void TCPClient::setup(std::string ip, short port = 12345)
{
    this->ip_addr = ip;
    this->port = port;
    struct sockaddr_in client;
    client.sin_family = AF_INET;
    client.sin_port = htons(port);
    client.sin_addr.s_addr = ::inet_addr(ip.c_str());

    {
        int time_out = 0;
        while (connect(sock, (struct sockaddr *)&client, sizeof(client)) != 0)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            time_out += 1;
            if (time_out % 20 == 0)
            {
                std::cout << "[" << time_out * 50 / 1000 << "] Checking the server [" << ip << ":" << port << "] has been opened" << std::endl;
            }
            if (time_out > 200 * 30)
            {
                std::cerr << "On connection error in " << 50 * time_out / 1000 << " s" << std::endl;
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
        this->send_message(this->sock, this->send_channel);
    }));
}

void TCPClient::start_service2()
{
    this->send_thread = std::make_shared<std::thread>(std::thread([this]() {
        this->send_message(this->sock, this->send_channel);
    }));
    this->recv_thread = std::make_shared<std::thread>(std::thread([this]() {
        this->recv_message(this->sock);
    }));
}

void TCPClient::send_message(int socket_fd, BlockingQueue<int> *queue)
{
    size_t max_bytes = 1000 * 1000 * 100 * sizeof(float);
    char *buff = new char[max_bytes];

    if (buff == NULL || buff == nullptr)
    {
        std::cout << "Error in malloc memory buff to send" << std::endl;
        exit(0);
    }
    //while (true)
    {
        std::cout << " in send message blocks" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    if (queue == nullptr)
    {
        std::cout << "error in send_message, send queue is null" << std::endl;
        exit(-1);
    }
    //while (true)
    {
        std::cout << "After in send message blocks" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    int size_to_send = 0;
    int size_sent = 0;
    //queue->push(1000 * 1000);
    do
    {
        size_to_send = queue->pop();

        while (size_to_send > 0)
        {
            size_sent = write(socket_fd, buff, size_to_send);
            if (size_sent <= 0)
            {
                std::cout << "send size is not right: " << size_sent << std::endl;
            }
            size_to_send -= size_sent;
        }
        if (size_to_send < 0)
        {
            std::cout << "error in send_message, send size: " << size_to_send << std::endl;
            exit(-1);
        }
    } while (true);
}

int TCPClient::recv_message(int socket_fd)
{
    size_t buff_size = 1000 * 1000 * 100 * sizeof(float);
    char *buff = new char[buff_size];
    size_t received = 0;
    Timer timer;
    timer.start();
    do
    {
        //std::cout << "Client recv from server" << std::endl;
        int recv_size = 0;
        recv_size = read(socket_fd, buff, buff_size);
        if (recv_size < 0)
        {
            std::cout << "recv error: " << recv_size << std::endl;
        }
        received += recv_size;
        if (received >= 1000000000)
        {
            timer.stop();
            std::cout << "Client recv rate: " << 8 * received / 1000.0 / 1000 / 1000 / timer.seconds() << " Gbps" << std::endl;
            //this->callbacks();
            received %= 1000000000;
            timer.start();
        }
    } while (true);
}

void TCPClient::send_data(int data_size)
{
    if (this->send_channel == nullptr)
    {
        std::cout << "Error in TCP client, send channel is null" << std::endl;
    }
    this->send_channel->push(data_size);
}