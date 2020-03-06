#ifndef __NEWPLAN_DATA_GENERATOR_H__
#define __NEWPLAN_DATA_GENERATOR_H__

#include <thread>
#include <functional>
#include <iostream>
#include <vector>
#include "../utils/blockingQueue.h"
class DataGenerator
{
public:
    DataGenerator();
    ~DataGenerator();
    void setup_pattern(std::vector<std::vector<int>> &pattern);
    void setup_channel(std::vector<BlockingQueue<int> *> channel_);
    void run();
    //void setup();

private:
    std::shared_ptr<std::thread> compute_thread;
    std::vector<std::vector<int>> dataflow_pattern;
    std::function<void(void)> callbacks;
    std::vector<BlockingQueue<int> *> channels;
};

#endif