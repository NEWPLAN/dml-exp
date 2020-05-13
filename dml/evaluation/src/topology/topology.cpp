#include "topology.h"
#include <iostream>
#include <fstream>

#include <stdio.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>

Topology::Topology(std::string path)
{
    this->ip_prefix = "12.12.12.1";
    this->prefix_length = 24;
    this->node_id = NetTool::get_node_id(ip_prefix.c_str(), prefix_length);
    ip_prefix = NetTool::get_ip_prefix(ip_prefix.c_str(), prefix_length);
    this->load_topology(path);
    LOG(INFO) << "Building Topology on " << node_id;
    LOG(INFO) << "Using ip prefix: " << ip_prefix;
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
            {
                if (this->root_id > index)
                {
                    this->root_id = index;
                }
                if (this->root_id > each_item)
                {
                    this->root_id = each_item;
                }
                involved_node++;
            }
        }
    this->root_id += 101;
    LOG(INFO) << "The root_id is: " << this->root_id;

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