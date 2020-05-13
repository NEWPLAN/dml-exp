#include <iostream>
#include "RDMABase.h"
#include "RDMAServer.h"
#include "RDMAClient.h"

void help(void)
{
    std::cout << "Useage:\n";
    std::cout << "For Server: ./rdma --server server_ip\n";
    std::cout << "For Client: ./rdma --server server_ip --client client_ip" << std::endl;
    return;
}

int single_benchmark(int argc, char const *argv[])
{
    RDMAAdapter rdma_adapter;

    RDMAClient *rclient;
    RDMAServer *rserver;

    switch (argc)
    {
    case 3:
        rdma_adapter.set_server_ip(argv[2]);

        rserver = new RDMAServer(rdma_adapter);
        rserver->setup();
    case 5:
        rdma_adapter.set_server_ip(argv[2]);
        rdma_adapter.set_client_ip(argv[4]);

        rclient = new RDMAClient(rdma_adapter);
        rclient->setup();
    default:
        help();
        exit(-1);
        break;
    }

    return 0;
}

#include <vector>
#include <string>

void server_functions(std::vector<std::string> ip)
{
    RDMAAdapter rdma_adapter;
    rdma_adapter.set_server_ip(ip[0].c_str());

    RDMAServer *rserver;

    rserver = new RDMAServer(rdma_adapter);
    rserver->setup();
    while (1)
    {
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }
}

void client_functions(std::vector<std::string> ip)
{
    if (ip.size() == 0)
    {
        std::cout << "Error of local IP" << std::endl;
    }
    std::string local_ip = ip[0];
    for (size_t i = 1; i < ip.size(); i++)
    {
        std::string server_ip = ip[i];

        std::cout << "Connecting to: " << server_ip << std::endl;
        new std::thread([server_ip, local_ip]() {
            RDMAClient *rclient;
            RDMAAdapter rdma_adapter;

            rdma_adapter.set_server_ip(server_ip.c_str());
            rdma_adapter.set_client_ip(local_ip.c_str());
            rclient = new RDMAClient(rdma_adapter);
            rclient->setup();
        });
    }
    while (1)
    {
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }
}

int main(int argc, char const *argv[])
{ //./main self-ip, remote ip
    if (argc < 2)
    {
        help();
    }
    else if (strcmp(argv[1], "--server") == 0)
    {
        single_benchmark(argc, argv);
    }

    std::vector<std::string> ip_address;
    for (int index = 1; index < argc; index++)
    {
        std::string ip = std::string(argv[index]);
        std::cout << "IP: " << ip << std::endl;
        ip_address.push_back(ip);
    }

    std::thread *server_thread; //
    std::thread *client_thread;
    server_thread = new std::thread([ip_address]() {
        server_functions(ip_address);
    });
    client_thread = new std::thread([ip_address]() {
        client_functions(ip_address);
    });
    server_thread->join();
    client_thread->join();

    return 0;
}