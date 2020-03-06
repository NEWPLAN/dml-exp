#include "data_generator.h"
#include "../../utils/blockingQueue.h"

int main(int argc, char **argv)
{
    BlockingQueue<int> control_channel, data_channel;
    std::vector<BlockingQueue<int> *> channels;
    channels.push_back(&control_channel);
    channels.push_back(&data_channel);

    std::vector<std::vector<int>> patterns;
    for (int index = 1; index < 10; index++)
    {
        std::vector<int> data;
        data.push_back(index * 100000);
        data.push_back(index);
        patterns.push_back(std::move(data));
    }

    DataGenerator *data_generator = new DataGenerator();
    //data_generator->setup(patterns, );
    data_generator->setup_pattern(patterns);
    data_generator->setup_channel(channels);
    data_generator->run();
    do
    {
        control_channel.push(1);
        int data = data_channel.pop();
        std::cout << "Receive data: " << data << std::endl;
    } while (true);
}