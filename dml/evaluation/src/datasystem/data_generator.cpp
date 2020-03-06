#include <data_generator.h>

DataGenerator::DataGenerator()
{
    this->callbacks = []() {
        std::cout << "Default data generator callbacks" << std::endl;
    };
    std::cout << "Building data generator..." << std::endl;
}
DataGenerator::~DataGenerator()
{
    std::cout << "Destroying data generator..." << std::endl;
}

void DataGenerator::setup_pattern(std::vector<std::vector<int>> &pattern)
{
    this->dataflow_pattern = pattern;
}
void DataGenerator::setup_channel(std::vector<BlockingQueue<int> *> channel_)
{
    this->channels = channel_;
}

void DataGenerator::run()
{
    this->compute_thread = std::make_shared<std::thread>([this]() {
        auto channel_in = this->channels[0];
        auto channel_out = this->channels[1];
        do
        {
            int ready = channel_in->pop();
            if (ready == -1)
                break;
            for (auto interval : this->dataflow_pattern)
            {
                std::this_thread::sleep_for(std::chrono::microseconds(interval[0]));
                channel_out->push(interval[1]);
            }

        } while (true);
    });
}
