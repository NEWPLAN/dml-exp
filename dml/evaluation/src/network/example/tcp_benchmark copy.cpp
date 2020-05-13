#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

#define MAXLINE 4096 * 1000 * 10
#include <thread>
#include <string>
#include <vector>
#include <mutex>
#include <iostream>

std::mutex mtx;

int server_function(std::vector<std::string> ip_address)
{
    int listenfd, n;
    struct sockaddr_in servaddr;

    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        printf("create socket error: %s(errno: %d)\n", strerror(errno), errno);
        return 0;
    }
    int on = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on));
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEPORT, (char *)&on, sizeof(on));

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(6666);

    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1)
    {
        printf("bind socket error: %s(errno: %d)\n", strerror(errno), errno);
        return 0;
    }

    if (listen(listenfd, 10) == -1)
    {
        printf("listen socket error: %s(errno: %d)\n", strerror(errno), errno);
        return 0;
    }
    int index = 0;
    size_t len = 0;
    size_t server_performance = 0;
    printf("========waiting for client's request========\n");
    std::thread *check_thread = new std::thread([&server_performance]() {
        do
        {
            std::this_thread::sleep_for(std::chrono::seconds(10));
            size_t all_data = 0;
            {
                std::lock_guard<std::mutex> lg(mtx);
                all_data = server_performance;
                server_performance = 0;
            }
            std::cout << "Total Performance: " << all_data * 8 / 1000.0 / 1000 / 1000 / 10 << " Gbps" << std::endl;
        } while (true);
    });

    while (index < ip_address.size())
    {
        int connfd;

        struct sockaddr_in client;
        socklen_t len = sizeof(client);

        if ((connfd = accept(listenfd, (struct sockaddr *)&client, &len)) == -1)
        {
            printf("accept socket error: %s(errno: %d)\n", strerror(errno), errno);
            exit(-1);
        }
        printf("[Server] received connection: %s:%d\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));
        index++;
        size_t *data_tp = &server_performance;
        std::thread *tp = new std::thread([connfd, data_tp]() {
            char *buff = new char[MAXLINE];
            int recv_num = 0;
            size_t recv_total = 0;

            std::cout << "received connection from: " << connfd << std::endl;

            int connfd_ = connfd;
            while (true)
            {
                recv_num = recv(connfd_, buff, MAXLINE, 0);
                if (recv_num <= 0)
                {
                    std::cout << "error of recv" << std::endl;
                    exit(-1);
                }
                recv_total += recv_num;
                if (recv_total > 200000000)
                {

                    //std::cout << "Recv total: " << recv_total << std::endl;
                    {
                        std::lock_guard<std::mutex> lg(mtx);
                        *data_tp += recv_total;
                        recv_total = 0;
                    }
                }
            }
        });
    }

    check_thread->join();
    return 0;
}

int client_function(std::vector<std::string> ip_address)
{
    for (auto &ip : ip_address)
    {
        std::cout << "Connecting to " << ip << std::endl;
        int sockfd, n;
        struct sockaddr_in servaddr;

        if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            printf("create socket error: %s(errno: %d)\n", strerror(errno), errno);
            return 0;
        }

        memset(&servaddr, 0, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(6666);
        servaddr.sin_addr.s_addr = ::inet_addr(ip.c_str());

        while (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
        {
            printf("connect error: %s(errno: %d)\n", strerror(errno), errno);
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        std::thread *tp = new std::thread([ip, sockfd]() {
            std::string ip_a = ip;

            char *send_line = new char[MAXLINE];
            std::cout << "Clinet is sending data to " << ip_a << std::endl;
            do
            {
                if (send(sockfd, send_line, MAXLINE, 0) < 0)
                {
                    printf("send msg error: %s(errno: %d)\n", strerror(errno), errno);
                    return 0;
                }
            } while (true);
        });
    }
    do
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    } while (true);
    return 0;
}

int main(int argc, char **argv)
{
    std::vector<std::string> ip_address;
    for (int index = 1; index < argc; index++)
    {
        std::string ip = std::string(argv[index]);
        std::cout << "IP: " << ip << std::endl;
        ip_address.push_back(ip);
    }
    std::thread *server, *client;
    server = new std::thread([ip_address]() {
        server_function(ip_address);
    });
    client = new std::thread([ip_address]() {
        client_function(ip_address);
    });

    do
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    } while (true);
    return 0;
}
