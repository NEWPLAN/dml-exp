#ifndef __NEWPLAN_TEST_H__
#define __NEWPLAN_TEST_H__

#include "../network/client.h"
#include "../network/server.h"
#include "../computation/tensor_merge.h"
#include <string>
class Solution
{
public:
    Solution();
    ~Solution();

    void run();
    void setup_network(std::string ip, short port);
    void setup_computation();

    void wait_for_done();

private:
    TCPClient *client;
    TCPServer *server;
    ComputeEngine *compute_engine;
};

#endif
