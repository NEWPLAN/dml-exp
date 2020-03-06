#ifndef __NEWPLAN_CUDA_COMPUTATION_H__
#define __NEWPLAN_CUDA_COMPUTATION_H__

void API_add_v2(float *a, float *b, float *c, int data_num);
int cuda_test_main(void);

#include <thread>
#include "../utils/thread_pool.hpp"
#include <vector>
#include <functional>
class ComputeEngine
{
public:
    ComputeEngine();
    ~ComputeEngine();
    void setup(int block_size, int tensor_num, int thread_pool_size);
    void run();

private:
    std::thread *compute_thread = nullptr;
    int block_size;
    int tensor_num;
    newplan::ThreadPool *tp = nullptr;
    std::vector<float *> data;
    std::function<void(void)> callbacks;
};

#endif