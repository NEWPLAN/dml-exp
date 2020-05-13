#include "topology.h"
#include <iostream>

int main(int argc, char **argv)
{
    Topology *topo = new Topology("/home/newplan/program/dml/evaluation/src/topology/topo.txt");
    //topo->load_topology();
    std::vector<int> upper = topo->get_upper_stream();
    std::vector<int> down = topo->get_down_stream();
    std::cout << "\nupper Stream: ";
    for (auto &index : upper)
    {
        std::cout << index << ", ";
    }
    std::cout << "\ndown Stream: ";
    for (auto &index : down)
    {
        std::cout << index << ", ";
    }
    std::cout << std::endl;
    delete topo;
    return 0;
}