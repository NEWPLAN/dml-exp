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
#include "../utils/ATimer.h"
#include "../utils/logging.h"

TCPServer::TCPServer()
{
    this->recv_channel = new BlockingQueue<int>();
    this->callbacks = []() { std::cout << "Default server callbacks after receive" << std::endl; };
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
    std::cout << "Server has been created" << std::endl;
}

TCPServer::~TCPServer()
{
    std::cout << "On closing server..." << std::endl;
}

BlockingQueue<int> *TCPServer::get_recv_channel()
{
    return this->recv_channel;
}
void TCPServer::setup(const std::string ip, short port)
{
    //socket打开一个 网络通讯端口，其中AF_INET:表示IPV4，SOCK_STREAM：表示面向流的传输，
    //protocol参数默认选择为0
    this->port = port;
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
    {
        //listen声明sock处于监听状态，并且最多允许5个客户端处于连接等待状态
        //如果收到更多的请求便忽略
        if (listen(this->server_socket, 100) == -1) //add listener
        {
            std::cerr << "On listen error" << std::endl;
            exit(-3);
        }
    }
}

void TCPServer::start_service(int service_number)
{
    std::cout << "Server is expected to accept " << service_number << " connections" << std::endl;
    if (connector_thread == nullptr)
    {
        connector_thread = new std::thread([this, service_number]() {
            std::vector<BlockingQueue<int> *> sub_channels;
            int served_number = 0;
            while (served_number < service_number)
            {
                served_number++;

                BlockingQueue<int> *tmp_channel = new BlockingQueue<int>();
                sub_channels.push_back(tmp_channel);

                struct sockaddr_in client;
                socklen_t len = sizeof(client);
                //accept阻塞式等待，用来接收连接
                int new_fd = accept(this->server_socket, (struct sockaddr *)&client, &len);
                if (new_fd < 0)
                {
                    std::cerr << "On accept error" << std::endl;
                    continue;
                }
                printf("[Server: %d] received a connection from %s, (all: %d)\n", this->port, inet_ntoa(client.sin_addr), served_number);

                worker_threads.push_back(new std::thread([this, new_fd, tmp_channel]() {
                    //this->recv_message(new_fd, tmp_channel);
                    recv_message_fix_size(new_fd, tmp_channel);
                }));
            }
            LOG(INFO) << "Server has received " << service_number
                      << " on port: " << this->port << ", chunk_size= " << chunk_size;

            int channel_size = sub_channels.size();
            size_t *recv_data_records = new size_t[channel_size];
            for (int index = 0; index < channel_size; index++)
            {
                // init the record for first read
                recv_data_records[index] = 0;
            }
            do
            {
                //fetch data from sub channels in round robin;
                for (int index = 0; index < channel_size; index++)
                {
                    while (recv_data_records[index] < chunk_size)
                    {
                        // if not data is available, wait for it
                        recv_data_records[index] += sub_channels[index]->pop();
                    }
                    recv_data_records[index] -= chunk_size;
                }
                // all subthread has received one block, then sent to forwarding engine
                this->recv_channel->push(chunk_size);
                this->event_signal->push(this->signal_);
            } while (true);
        });
    }
    else
    {
        std::cout << "Already create a connector" << std::endl;
    }
}

void TCPServer::start_service2()
{
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
                printf("[Server] received connection: %s:%d\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));

                worker_threads.push_back(new std::thread([this, new_fd](){
                    this->recv_message(new_fd,nullptr);
                }));
                worker_threads.push_back(new std::thread([this, new_fd](){
                    this->send_message(new_fd);
                }));
            } });
    }
    else
    {
        std::cout << "Already create a connector_thread" << std::endl;
    }
}

void TCPServer::send_message(int socket_fd)
{
    char buff[1024 * 1024];
    sprintf(buff, "%d", 1024 * 1024);
    do
    {
        //std::cout << "server send message to client" << std::endl;
        int send_size = 0;
        send_size = write(socket_fd, buff, 1024 * 1024);
        if (send_size != 1024 * 1024)
        {
            std::cout << "send size is not right: " << send_size << std::endl;
        }
    } while (true);
}

int TCPServer::recv_message(int socket_fd, BlockingQueue<int> *queue)
{
    size_t buff_size = 1024 * 1024 * 100;
    char *buff = new char[buff_size];
    size_t received = 0;
    Timer timer;
    timer.start();
    if (queue == nullptr)
    {
        std::cout << "error of the received message, no queue" << std::endl;
        exit(-1);
    }
    int to_receive_bytes = chunk_size;
    {
        std::cout << "Server [" << this->port << "] has been ready to receive message, chunk size = " << chunk_size << std::endl;
    }
    if (chunk_size <= 0)
    {
        std::cout << "error in recv chunk_size: " << chunk_size << std::endl;
        exit(0);
    }
    int recv_size = 0;
    do
    {
        //std::cout << "Server recv from client" << std::endl;

        //read, check, and put to channels
        recv_size = read(socket_fd, buff, buff_size);
        if (recv_size <= 0)
        {
            std::cout << "recv error, received: " << recv_size << std::endl;
            exit(-1);
        }
        received += recv_size;
        if (received >= 10000000000)
        {
            timer.stop();
            LOG(INFO) << "[" << this->port << "]" << socket_fd << ": Server recv rate: "
                      << 8 * received / 1000.0 / 1000 / 1000 / timer.seconds() << " Gbps";
            //this->callbacks();
            received %= 10000000000;
            timer.start();
        }
        //std::cout << "to_receive_bytes: " << to_receive_bytes << std::endl;
        queue->push(recv_size); // receive sub_thread-->receive main thread;
    } while (true);
}

int TCPServer::recv_message_fix_size(int socket_fd, BlockingQueue<int> *queue)
{
    size_t buff_size = 1024 * 1024 * 100;
    char *buff = new char[buff_size];
    size_t received = 0;
    Timer timer;
    timer.start();
    if (queue == nullptr)
    {
        std::cout << "error of the received message, no queue" << std::endl;
        exit(-1);
    }
    int to_receive_bytes = chunk_size;
    {
        std::cout << "Server [" << this->port << "] has been ready to receive message, chunk size = " << chunk_size << std::endl;
    }
    if (chunk_size <= 0)
    {
        std::cout << "error in recv chunk_size: " << chunk_size << std::endl;
        exit(0);
    }
    do
    {
        //std::cout << "Server recv from client" << std::endl;
        to_receive_bytes = chunk_size;
        int recv_size = 0;
        while (to_receive_bytes > 0)
        {
            //recv_size = read(socket_fd, buff, to_receive_bytes);
            recv_size = recv(socket_fd, buff, to_receive_bytes, MSG_WAITALL);

            if (recv_size <= 0)
            {
                std::cout << "recv error, received: " << recv_size << std::endl;
                exit(-1);
            }
            to_receive_bytes -= recv_size;
            //if (recv_size == 0)
            //std::cout << "received in bytes: " << recv_size << std::endl;

            received += recv_size;
            if (received >= 10000000000)
            {
                timer.stop();
                LOG(INFO) << "[" << this->port << "]" << socket_fd << ": Server recv rate: "
                          << 8 * received / 1000.0 / 1000 / 1000 / timer.seconds() << " Gbps";
                //this->callbacks();
                received %= 10000000000;
                timer.start();
            }
        }
        //std::cout << "to_receive_bytes: " << to_receive_bytes << std::endl;
        queue->push(chunk_size); // receive sub_thread-->receive main thread;

    } while (true);
}