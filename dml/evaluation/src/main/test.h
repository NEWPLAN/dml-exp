#ifndef __NEWPLAN_TEST_H__
#define __NEWPLAN_TEST_H__

// #include "../network/tcp/client.h"
// #include "../network/tcp/server.h"
#include "../computation/tensor_merge.h"
#include "../datasystem/data_generator.h"
#include "../computation/aggregator.h"
#include "../topology/topology.h"
#include "../network/tower.h"
#include <string>
#include "../utils/blockingQueue.h"
#include "../utils/logging.h"
class Benchmark
{
public:
    Benchmark(int chunk_size);
    ~Benchmark();

    void run();
    void setup_network(std::string ip, short port);
    void setup_dataGenetator();
    void setup_aggregator();
    void setup_topology();
    void forward_engine_start();
    void wait_for_done();

private:
    DataGenerator *data_generator;
    Aggregator *aggregator;
    Topology *topo;
    Tower *to_up_stream;
    Tower *to_down_stream;
    int node_id;
    int node_size;
    int chunk_size;
    BlockingQueue<int> event_siganl;
    std::vector<BlockingQueue<int> *> aggregator_channels;
};

#endif
