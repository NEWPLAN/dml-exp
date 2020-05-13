#ifndef __NEWPLAN_COORDINATOR_H__
#define __NEWPLAN_COORDINATOR_H__
#include "server.h"
#include "client.h"

class Coordinator
{
public:
    Coordinator(int node_id, int node_base, int rank_size);
    ~Coordinator();

private:
    int node_id;
    int node_base;
    TCPServer *server;
    TCPClient *client;
};

#endif