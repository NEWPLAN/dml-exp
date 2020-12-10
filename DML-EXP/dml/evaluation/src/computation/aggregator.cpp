#include "aggregator.h"
#include <iostream>
#include "../utils/ATimer.h"
#include <thread>

Aggregator::Aggregator()
{
    this->callbacks = []() { std::cout << "Default Aggregator callback after merging" << std::endl; };
    this->before_compute.push_back([]() { std::cout << "Default Aggregator callback before merging" << std::endl; });
    this->after_compute.push_back([]() { std::cout << "Default Aggregator callback after merging" << std::endl; });

    std::cout << "Aggregator has been ctreated" << std::endl;
}
Aggregator::~Aggregator()
{
    delete compute_thread;
    for (int index = 0; index < data_groups.size(); index++)
    {
        for (auto &data_ptr : data_groups[index])
        {
            delete data_ptr;
        }
    }
    std::cout << "Destroying Aggregator engine" << std::endl;
}
void Aggregator::setup(int block_size, int tensor_num, int thread_pool_size)
{
    this->block_size = block_size;
    this->tensor_num = tensor_num;
    this->thread_pool_size_ = thread_pool_size;

    this->threadpool_context = new newplan::ThreadPool(thread_pool_size);
    std::cout << "Creating thread pool: " << thread_pool_size << std::endl;

    for (int data_round = 0; data_round < 3; data_round++)
    {
        std::vector<float *> data_unit;
        for (int tensor_index = 0; tensor_index < tensor_num + 1; tensor_index++)
        {
            float *tmp_data = new float[block_size + 10];
            if (tmp_data == nullptr)
            {
                std::cerr << "Error in malloc data" << std::endl;
                exit(-1);
            }
            data_unit.push_back(tmp_data);
        }
        this->data_groups.push_back(std::move(data_unit));
    }

    std::cout << "Malloc CPU memory size: " << block_size << " B * " << tensor_num + 1 << ", with backups: " << this->data_groups.size() << std::endl;
}

static void reduce(std::vector<float *> &data_, int num_tensor, int start, int stop)
{
    float *result = data_[num_tensor];
    float *data[160];
    for (int index = 0; index < data_.size(); index++)
        data[index] = data_[index];
    switch (num_tensor)
    {
    case 1:
        for (int i = start; i < stop; i++)
            result[i] = data[0][i];
        break;
    case 2:
        for (int i = start; i < stop; i++)
            result[i] = data[0][i] + data[1][i];
        break;
    case 3:
        for (int i = start; i < stop; i++)
            result[i] = data[0][i] + data[1][i] + data[2][i];
        break;
    case 4:
        for (int i = start; i < stop; i++)
            result[i] = data[0][i] + data[1][i] + data[2][i] + data[3][i];
        break;
    case 5:
        for (int i = start; i < stop; i++)
            result[i] = data[0][i] + data[1][i] + data[2][i] + data[3][i] + data[4][i];
        break;
    case 6:
        for (int i = start; i < stop; i++)
            result[i] = data[0][i] + data[1][i] + data[2][i] + data[3][i] + data[4][i] + data[5][i];
        break;
    case 7:
        for (int i = start; i < stop; i++)
            result[i] = data[0][i] + data[1][i] + data[2][i] + data[3][i] + data[4][i] + data[5][i] + data[6][i];
        break;
    case 8:
        for (int i = start; i < stop; i++)
            result[i] = data[0][i] + data[1][i] + data[2][i] + data[3][i] + data[4][i] + data[5][i] + data[6][i] + data[7][i];
        break;
    case 9:
        for (int i = start; i < stop; i++)
            result[i] = data[0][i] + data[1][i] + data[2][i] + data[3][i] + data[4][i] + data[5][i] + data[6][i] + data[7][i] + data[8][i];
        break;
    case 10:
        for (int i = start; i < stop; i++)
            result[i] = data[0][i] + data[1][i] + data[2][i] + data[3][i] + data[4][i] + data[5][i] + data[6][i] + data[7][i] + data[8][i] + data[9][i];
        break;
    case 11:
        for (int i = start; i < stop; i++)
            result[i] = data[0][i] + data[1][i] + data[2][i] + data[3][i] + data[4][i] + data[5][i] + data[6][i] + data[7][i] + data[8][i] + data[9][i] + data[10][i];
        break;
    case 12:
        for (int i = start; i < stop; i++)
            result[i] = data[0][i] + data[1][i] + data[2][i] + data[3][i] + data[4][i] + data[5][i] + data[6][i] + data[7][i] + data[8][i] + data[9][i] + data[10][i] + data[11][i];
        break;
    case 13:
        for (int i = start; i < stop; i++)
            result[i] = data[0][i] + data[1][i] + data[2][i] + data[3][i] + data[4][i] + data[5][i] + data[6][i] + data[7][i] + data[8][i] + data[9][i] + data[10][i] + data[11][i] + data[12][i];
        break;
    case 14:
        for (int i = start; i < stop; i++)
            result[i] = data[0][i] + data[1][i] + data[2][i] + data[3][i] + data[4][i] + data[5][i] + data[6][i] + data[7][i] + data[8][i] + data[9][i] + data[10][i] + data[11][i] + data[12][i] + data[13][i];
        break;
    case 15:
        for (int i = start; i < stop; i++)
            result[i] = data[0][i] + data[1][i] + data[2][i] + data[3][i] + data[4][i] + data[5][i] + data[6][i] + data[7][i] + data[8][i] + data[9][i] + data[10][i] + data[11][i] + data[12][i] + data[13][i] + data[14][i];
        break;
    case 16:
        for (int i = start; i < stop; i++)
            result[i] = data[0][i] + data[1][i] + data[2][i] + data[3][i] + data[4][i] + data[5][i] + data[6][i] + data[7][i] + data[8][i] + data[9][i] + data[10][i] + data[11][i] + data[12][i] + data[13][i] + data[14][i] + data[15][i];
        break;
    default:
        std::cout << "error in unknown tensors: " << num_tensor << std::endl;
        break;
    }
}

void Aggregator::setup_channels(std::vector<BlockingQueue<int> *> channels)
{
    this->channels_ = channels;
}

void Aggregator::run()
{
    daemon_thread = new std::thread([this]() {
        int data_round = 0;
        do
        {
            int tensor_num_ = channels_[0]->pop();
            int tensor_size_ = channels_[0]->pop();
            if (tensor_size_ > 100000000)
            {
                std::cout << "too much tensor size: " << tensor_size_ << std::endl;
                exit(-1);
            }
            if (tensor_num_ > 15)
            {
                std::cout << "too many tensor num: " << tensor_num_ << std::endl;
                exit(-1);
            }

//std::cout << "Merging tensor here!" << std::endl;
#if defined DEBUG_AGGREGATOR
            Timer t1;
            t1.start();
#endif
            {
                for (int start = 0; start < tensor_size_; start += tensor_size_ / this->thread_pool_size_)
                {
                    int stop = start + tensor_size_ / this->thread_pool_size_;

                    this->threadpool_context->runTask([this, data_round, start, stop]() {
                        reduce(this->data_groups[data_round], this->tensor_num, start, stop);
                    });
                }
                this->threadpool_context->waitWorkComplete();
            }
#if defined DEBUG_AGGREGATOR
            t1.stop();
            std::cout << "num tensor: " << tensor_num_ << ", size: " << tensor_size_ / 1000 << " KB, Time cost: " << t1.milliseconds() << " ms" << std::endl;
#endif

            data_round = (data_round + 1) % 3;

            channels_[1]->push(tensor_size_);
            this->event_signal->push(1); //send signal to forward engine
        } while (true);
    });
}

int Aggregator::receive_message(int size)
{
    return channels_[1]->pop();
}
void Aggregator::send_message(int block_nums, int data_size)
{
    channels_[0]->push(block_nums);
    channels_[0]->push(data_size);
}