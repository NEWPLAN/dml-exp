#ifndef __NEWPLAN_TOPOLOGY_H__
#define __NEWPLAN_TOPOLOGY_H__

#include <vector>
#include <string>
#include "../utils/logging.h"
#include "../utils/NetUtil.hpp"

class Topology
{
public:
    explicit Topology(std::string path);
    ~Topology();
    void load_topology(std::string path);

    std::vector<int> get_upper_stream();
    std::vector<int> get_down_stream();
    int get_id() { return node_id; }
    int get_rank_size() { return involved_node; }
    std::string get_root_ip()
    {
        if (this->root_id == 256)
        {
            LOG(INFO) << "Get root id failed";
            exit(-1);
        }
        return ip_prefix + std::to_string(this->root_id);
    }

private:
    int **adj_matrix = nullptr;
    int node_id = -1; //id for this node
    int node_size = 0;
    std::string file_path;
    int involved_node = 0;

    int root_id = 256;
    std::string ip_prefix;
    int prefix_length = 0;
};

#endif