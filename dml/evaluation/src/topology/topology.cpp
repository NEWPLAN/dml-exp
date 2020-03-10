#include "topology.h"
#include <iostream>
#include <fstream>

#include <stdio.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>

static int get_local_id(const char *ip_prefix, int ip_prefix_length)
{
    struct ifaddrs *ifAddrStruct = NULL;
    struct ifaddrs *ifa = NULL;
    void *tmpAddrPtr = NULL;

    int node_id = -1;

    getifaddrs(&ifAddrStruct);

    for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next)
    {
        if (!ifa->ifa_addr)
        {
            continue;
        }
        if (ifa->ifa_addr->sa_family == AF_INET) // check it is IP4
        {
            // is a valid IP4 Address
            tmpAddrPtr = &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
            char addressBuffer[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
            if (strncmp(addressBuffer, ip_prefix, ip_prefix_length) == 0)
            {
                node_id = atoi(addressBuffer + ip_prefix_length);
                printf("%s IP Address %s\n", ifa->ifa_name, addressBuffer);
            }
        }
        else if (ifa->ifa_addr->sa_family == AF_INET6) // check it is IP6
        {
            continue;
            // is a valid IP6 Address
            tmpAddrPtr = &((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_addr;
            char addressBuffer[INET6_ADDRSTRLEN];
            inet_ntop(AF_INET6, tmpAddrPtr, addressBuffer, INET6_ADDRSTRLEN);
            printf("%s IP Address %s\n", ifa->ifa_name, addressBuffer);
        }
    }
    if (ifAddrStruct != NULL)
    {
        freeifaddrs(ifAddrStruct);
    }

    return node_id;
}

Topology::Topology()
{
    this->node_id = get_local_id("12.12.12.111", 9);
    std::cout << "Building Topology ..." << std::endl;
}

Topology::~Topology()
{
    std::cout << "Destroying Topology..." << std::endl;
}
void Topology::load_topology(std::string path)
{
    std::cout << "Loading Topology..." << std::endl;
    if (path.empty())
        path = "./topo.txt";
    this->file_path = path;
    std::ifstream topo_reader(path, std::ios::in);
    if (!topo_reader)
    {
        std::cout << "Error in open file: " << path << std::endl;
        exit(-1);
    }
    int node_size = 0;
    topo_reader >> node_size;
    this->node_size = node_size;
    int temp_id = 0;
    std::cout << "maximum node number: " << node_size << std::endl;
    for (int index = 0; index < node_size; index++)
    {
        topo_reader >> temp_id;
    }
    this->adj_matrix = new int *[node_size];
    for (int index = 0; index < node_size; index++)
    {
        this->adj_matrix[index] = new int[node_size];
        topo_reader >> temp_id;
        for (int each_item = 0; each_item < node_size; each_item++)
        {
            topo_reader >> temp_id;
            this->adj_matrix[index][each_item] = temp_id;
        }
        this->adj_matrix[index][index] = 0;
    }
    involved_node = 0;
    for (int index = 0; index < node_size; index++)
        for (int each_item = 0; each_item < node_size; each_item++)
        {
            if (this->adj_matrix[index][each_item] != 0)
                involved_node++;
        }

    // for (int index = 0; index < node_size; index++)
    // {
    //     for (int each_item = 0; each_item < node_size; each_item++)
    //         std::cout << this->adj_matrix[index][each_item] << ", ";
    //     std::cout << std::endl;
    // }
}

std::vector<int> Topology::get_upper_stream()
{
    std::cout << "Getting upper stream..." << std::endl;
    std::vector<int> uper;
    int base_idx = this->node_id - 101;
    for (int index = 0; index < node_size; index++)
    {
        if (this->adj_matrix[base_idx][index] == 1)
            uper.push_back(index + 101);
    }
    return uper;
}
std::vector<int> Topology::get_down_stream()
{
    std::cout << "Getting down stream..." << std::endl;

    std::vector<int> lower;
    int base_idx = this->node_id - 101;
    for (int index = 0; index < node_size; index++)
    {
        if (this->adj_matrix[index][base_idx] == 1)
            lower.push_back(index + 101);
    }
    return lower;
}