#include "tensor_merge.h"
#include <iostream>
#include "../utils/ATimer.h"
#include <thread>

ComputeEngine::ComputeEngine()
{
    this->callbacks = []() { std::cout << "Default ComputeEngine callback after merging" << std::endl; };
    std::cout << "ComputeEngine has been ctreated" << std::endl;
}
ComputeEngine::~ComputeEngine()
{
    delete compute_thread;
    for (auto &data_ptr : data)
    {
        delete data_ptr;
    }
    std::cout << "Destroying computation engine" << std::endl;
}
void ComputeEngine::setup(int block_size, int tensor_num, int thread_pool_size)
{
    this->block_size = block_size;
    this->tensor_num = tensor_num;
    tp = new newplan::ThreadPool(thread_pool_size);
    for (int num = 0; num < tensor_num + 1; num++)
    {
        float *tmp_data = new float[block_size + 10];
        if (tmp_data == nullptr)
        {
            std::cerr << "Error in malloc data" << std::endl;
        }
        data.push_back(tmp_data);
    }
    std::cout << "Malloc CPU memory size: " << block_size << "*" << tensor_num + 1 << std::endl;
}

static void tensor_merge(float *result, float **data, int num_tensor, int start, int stop)
{
    for (int i = start; i < stop; i++)
    {
        switch (num_tensor)
        {
        case 2:
            result[i] = data[0][i] + data[1][i];
            break;
        case 3:
            result[i] = data[0][i] + data[1][i] + data[2][i];
            break;
        case 4:
            result[i] = data[0][i] + data[1][i] + data[2][i] + data[3][i];
            break;
        case 5:
            result[i] = data[0][i] + data[1][i] + data[2][i] + data[3][i] + data[4][i];
            break;
        default:
            std::cout << "error tensor number: " << num_tensor << std::endl;
        }
    }
}

static void do_test(newplan::ThreadPool *context, float *result, float **data, int num_tensor, int size)
{
    for (int nums = 0; nums < size; nums += size / 20)
    {
        int end_posi = nums + size / 20;
        context->runTask([result, data, num_tensor, nums, end_posi]() {
            tensor_merge(result, (float **)data, num_tensor, nums, end_posi);
        });
    }
    context->waitWorkComplete();
    return;
}
static void do_init(newplan::ThreadPool *context, int *data, int data_size)
{
    for (size_t i = 0; i < 20; i++)
    {
        context->runTask([data, data_size]() {for (int i=0;i<data_size;i++)
		{
			data[i]=(data[i]%191)*(data[i]%99983);
		} });
    }
    context->waitWorkComplete();
    return;
}

void ComputeEngine::run()
{
    this->compute_thread = new std::thread([this]() {
        float *data_[this->tensor_num];
        for (int num = 0; num < this->tensor_num; num++)
        {
            data_[num] = this->data[num];
        }

        std::cout << "Run background computation thread." << std::endl;

        int *ramdom = new int[1024 * 1024 * 100];
        int random_size = 1024 * 1024 * 100;
        for (int data_size = 1000; data_size <= block_size; data_size *= 10)
        {
            for (int ten_num = 2; ten_num <= tensor_num; ten_num++)
            {
                for (int loops = 0; loops < 5; loops++)
                {
                    Timer t1;
                    t1.start();
                    {
                        do_test(tp, this->data[tensor_num], data_, ten_num, data_size);
                    }
                    t1.stop();
                    std::cout << "num tensor: " << ten_num << ", size: " << data_size / 1000 << " KB, Time cost: " << t1.milliseconds() << " ms" << std::endl;

                    {
                        do_init(tp, ramdom, random_size);
                        this->callbacks();
                    }
                }
            }
        }
        while (1)
            ;
    });
}