#ifndef __NEWPLAN_TOPOLOGY_H__
#define __NEWPLAN_TOPOLOGY_H__

#include <vector>
#include <string>

class Topology
{
public:
    Topology();
    ~Topology();
    void load_topology(std::string path);

    std::vector<int> get_upper_stream();
    std::vector<int> get_down_stream();
    int get_id() { return node_id; }
    int get_rank_size() { return involved_node; }

private:
    int **adj_matrix = nullptr;
    int node_id = -1; //id for this node
    int node_size = 0;
    std::string file_path;
    int involved_node = 0;
};

#endif