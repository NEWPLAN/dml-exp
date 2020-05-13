#include "coordinator.h"
#include <string>
#include "../utils/logging.h"

Coordinator::Coordinator(int node_id, int node_base, int rank_size)
{
    if (node_id == node_base)
    {
        std::string ip_addr = std::to_string(node_base);
        server = new TCPServer();
        server->setup(ip_addr, 2020);
        server->start_service();
        LOG(INFO) << "Run as master node";
    }
    else
    {
        client = new TCPClient();
        LOG(INFO) << "Run as slaver node";
    }
}
Coordinator::~Coordinator()
{
}