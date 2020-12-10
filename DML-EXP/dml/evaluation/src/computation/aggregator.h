#ifndef __NEWPLAN_AGGREGATOR_H__
#define __NEWPLAN_AGGREGATOR_H__
#include <vector>
#include <thread>
#include "../utils/thread_pool.hpp"
#include "../utils/blockingQueue.h"

class Aggregator
{
public:
    Aggregator();
    ~Aggregator();
    void setup(int block_size, int tensor_num, int thread_pool_size);
    void run();
    void setup_channels(std::vector<BlockingQueue<int> *> channels);
    void register_signal_event(BlockingQueue<int> *signal_queue) { this->event_signal = signal_queue; }

    int receive_message(int size);
    void send_message(int block_nums, int data_size);

private:
    std::thread *compute_thread = nullptr;
    std::thread *daemon_thread = nullptr;
    int block_size;
    int tensor_num;
    int thread_pool_size_;
    newplan::ThreadPool *threadpool_context = nullptr;
    std::vector<std::vector<float *>> data_groups;

    std::function<void(void)> callbacks;

    std::vector<std::function<void(void)>> before_compute;
    std::vector<std::function<void(void)>> after_compute;

    std::vector<BlockingQueue<int> *> channels_;

    BlockingQueue<int> *event_signal;
};

#endif