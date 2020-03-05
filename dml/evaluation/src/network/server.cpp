/*************************************************************************
        > Copyright(c)  NEWPLAN, all rights reserved.
        > File Name   : server.c
        > Author      : NEWPLAN
        > Mail        : newplan001@163.com
        > Created Time: 2018年11月06日 星期二 16时21分51秒
 ************************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include <iostream>
#include "server.h"
#include <thread>
#include <fcntl.h>

TCPServer::TCPServer()
{
    //socket打开一个 网络通讯端口，其中AF_INET:表示IPV4，SOCK_STREAM：表示面向流的传输，
    //protocol参数默认选择为0
    this->server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (this->server_socket == -1)
    {
        std::cerr << "Creating socket error" << std::endl;
        exit(-1);
    }
    int on = 1;
    ::setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on));
    ::setsockopt(server_socket, SOL_SOCKET, SO_REUSEPORT, (char *)&on, sizeof(on));
    std::cout << "On creating server..." << std::endl;
}

TCPServer::~TCPServer()
{
    std::cout << "On closing server..." << std::endl;
}

void TCPServer::setup(const std::string ip, short port)
{
    //socket打开一个 网络通讯端口，其中AF_INET:表示IPV4，SOCK_STREAM：表示面向流的传输，
    //protocol参数默认选择为0

    struct sockaddr_in local;
    local.sin_family = AF_INET;
    local.sin_port = htons(port);
    local.sin_addr.s_addr = inet_addr(ip.c_str());
    //bind的作用是将参数sock与local绑定在一起
    //使sock这个文件描述符监听local所描述的地址与端口号
    //成功返回0，失败返回-1
    if (bind(this->server_socket, (struct sockaddr *)&local, sizeof(local)) < 0) //success zero is return
    {
        std::cerr << "Binding socket error" << std::endl;
        exit(-2);
    }
}

void TCPServer::start_service(int service_number)
{
    //listen声明sock处于监听状态，并且最多允许5个客户端处于连接等待状态
    //如果收到更多的请求便忽略
    if (listen(this->server_socket, service_number) == -1) //add listener
    {
        std::cerr << "On listen error" << std::endl;
        exit(-3);
    }
    std::cout << "On listen maximum connections: " << service_number << std::endl;
    if (connector_thread == nullptr)
    {
        connector_thread = new std::thread([this]() {
            while (1)
            {
                struct sockaddr_in client;
                socklen_t len = sizeof(client);
                //accept阻塞式等待，用来接收连接
                int new_fd = accept(this->server_socket, (struct sockaddr *)&client, &len);
                if (new_fd < 0)
                {
                    std::cerr << "On accept error" << std::endl;
                    continue;
                }
                printf("get a new client,%s:%d\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));
                std::string client_ip=std::string(inet_ntoa(client.sin_addr));
                short client_port=ntohs(client.sin_port);
                worker_threads.push_back(new std::thread([this, new_fd](){
                    this->handle_request(new_fd);
                }));
                //return 0;
            } });
        // connector_thread->join();
    }
    else
    {
        std::cout << "Already create a connector_thread" << std::endl;
    }
}

void TCPServer::handle_request(int fd)
{
    int new_fd = fd;
    printf("new_fd=%d\n", new_fd);
    while (1)
    {
        char buffer[1024 * 1024];
        ssize_t s = read(new_fd, buffer, sizeof(buffer));
        if (s == -1)
        {
            perror("read");
        }
        if (s > 0)
        {

            buffer[s] = '\0';
            sprintf(buffer, "%ld", s);
            //printf("client:%s\n", buffer);
            write(new_fd, buffer, sizeof(int) + 1);
        }
        else
        {
            printf("read done...break\n");
            break;
        }
    }
}
