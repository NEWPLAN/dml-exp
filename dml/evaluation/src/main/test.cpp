#include "test.h"
#include <iostream>

Benchmark::Benchmark(int chunk_size)
{
    this->chunk_size = chunk_size;
    std::cout << "\n\n======= Building Benchmark =======" << std::endl;
    data_generator = new DataGenerator();
    aggregator = new Aggregator();
    topo = new Topology("/home/newplan/program/dml/evaluation/src/topology/topo.txt");

    to_up_stream = new Tower(UPPER_STREAM);
    to_down_stream = new Tower(DOWN_STREAM);
    {
        to_down_stream->setup_chunk_size(this->chunk_size);
        to_up_stream->setup_chunk_size(this->chunk_size);
    }

    {
        data_generator->register_signal_event(&(this->event_siganl));
        aggregator->register_signal_event(&(this->event_siganl));
        to_up_stream->register_signal_event(&(this->event_siganl));
        to_down_stream->register_signal_event(&(this->event_siganl));
    }

    LOG(INFO) << "Test case has been built on node: " << topo->get_id()
              << ", and the root ip is: " << topo->get_root_ip();
    //while (1)
    {
        std::cout << "Back to main thread in building benchmark" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    std::cout << "\n\n";
}
Benchmark::~Benchmark()
{
    std::cout << "Destroy Benchmark" << std::endl;
}

void Benchmark::run()
{
    // do
    // {
    //     std::cout << "receive_message from down: ";
    //     to_down_stream->receive_message(0);
    //     std::cout << "receive_message from upper: ";
    //     to_up_stream->receive_message(0);
    // } while (true);

    //server->start_service();
    std::cout << "Server has been serving" << std::endl;

    //client->start_service();
    std::cout << "Client has been serving" << std::endl;

    BlockingQueue<int> *control_channel_ = new BlockingQueue<int>();
    BlockingQueue<int> *data_channel_ = new BlockingQueue<int>();

    aggregator_channels.push_back(data_channel_);
    aggregator_channels.push_back(control_channel_);
    aggregator->setup_channels(aggregator_channels);

    aggregator->run();
    std::cout << "ComputeEngine has been serving" << std::endl;
    //do
    if (0)
    {
        for (int num_tensor = 2; num_tensor <= 15; num_tensor++)
        {
            for (int block_size = 1000; block_size <= 100000000; block_size *= 10)
            {
                data_channel_->push(num_tensor);
                data_channel_->push(block_size);
            }
        }
        //control_channel_->pop();

    } //while (1);

    data_generator->run();
    std::cout << "DataGenerator has been serving" << std::endl;

    std::cout << "All modules has been launched" << std::endl;
}

#include "../network/tower.h"
void Benchmark::setup_network(std::string ip, short port)
{
    std::cout << "=========setup network system=========="
              << std::endl;

    //topo->load_topology();

    {
        this->node_size = topo->get_rank_size();
        if (this->node_size <= 0 || topo->get_upper_stream().size() <= 0)
        {
            std::cout << "This node is not ready to join cluster" << std::endl;
            exit(-1);
        }
        this->node_size /= topo->get_upper_stream().size();
        if (node_size <= 0)
        {
            std::cout << "error of cluster configuration, node size: " << node_size << std::endl;
            exit(-1);
        }
        this->chunk_size = (this->chunk_size + this->node_size - 1) / this->node_size;

        std::cout << "Node size: " << this->node_size << ", chunk size: " << this->chunk_size << std::endl;
        to_down_stream->setup_chunk_size(this->chunk_size);
        to_up_stream->setup_chunk_size(this->chunk_size);
    }

    std::vector<std::string> ip_upper;
    std::vector<std::string> ip_down;
    for (auto up : topo->get_upper_stream())
    {
        std::string ip_address = "12.12.12." + std::to_string(up);
        ip_upper.push_back(ip_address);
        std::cout << "next upper_stream: " << ip_address << std::endl;
    }
    for (auto down : topo->get_down_stream())
    {
        std::string ip_address = "12.12.12." + std::to_string(down);
        ip_down.push_back(ip_address);
        std::cout << "next down_stream: " << ip_address << std::endl;
    }

    std::cout << "-------After network system----------" << std::endl;
    //while (1)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << "main thread in setup_network" << std::endl;
    }
    to_down_stream->build_network_system(ip_down, 1345);
    to_up_stream->build_network_system(ip_upper, 1344);

    to_down_stream->start_service();
    std::cout << "Launching down_stream tower" << std::endl;

    to_up_stream->start_service();
    std::cout << "Launching upper_stream tower" << std::endl;
    //while (1)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << "main thread is leaving setup_network" << std::endl;
    }
}

void Benchmark::setup_aggregator()
{
    aggregator->setup(100 * 1000 * 1000, 15, 10);
}

void Benchmark::setup_dataGenetator()
{
    LOG(INFO) << "setup dataGenerator";

    //BlockingQueue<int> control_channel, data_channel;
    BlockingQueue<int> *control_channel = new BlockingQueue<int>();
    BlockingQueue<int> *data_channel = new BlockingQueue<int>();
    std::vector<BlockingQueue<int> *> channels;
    channels.push_back(control_channel);
    channels.push_back(data_channel);

    std::vector<std::vector<int>> patterns;
    for (int index = 1; index < 10; index++)
    {
        std::vector<int> data;
        data.push_back(index * 100000);
        data.push_back(index);
        patterns.push_back(std::move(data));
    }

    data_generator->setup_pattern(patterns);
    data_generator->setup_channel(channels);
    if (0)
    {
        auto switch_thread = new std::thread([channels]() {do
        {
            channels[0]->push(1);
            int data = channels[1]->pop();
            LOG(INFO) << "Receive data: " << data ;
        } while (true); });
    }
}

void Benchmark::wait_for_done()
{
    LOG(INFO) << "Main thread is running in background";
    do
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    } while (1);
}

#include "../utils/ATimer.h"

void Benchmark::forward_engine_start()
{
    int message_signal = -1;
    int received_data = 0;
    //for (int index = 0; index < 10; index++)
    {
        to_up_stream->send_message(this->chunk_size);
    }

    // to_down_stream->send_message(1000000);
    // to_down_stream->send_message(1000000);
    // to_down_stream->send_message(1000000);
    // to_down_stream->send_message(1000000);
    // to_down_stream->send_message(1000000);
    // to_down_stream->send_message(1000000);
    // to_down_stream->send_message(1000000);
    // to_down_stream->send_message(1000000);
    // to_down_stream->send_message(1000000);
    // to_down_stream->send_message(1000000);
    {
        LOG(INFO) << "In forward_engine, node_size=" << this->node_size
                  << ", chunk size: " << this->chunk_size;
    }
    Timer timer;
    timer.start();
    size_t bytes_received = 0;
    int reduce_scatter = 0;
    int upper_node_num = topo->get_upper_stream().size();

    int is_ring = upper_node_num == 1 ? 1 : 0;
    if (!is_ring)
    {
        LOG(INFO) << "PS will pipeline!";
        for (int i = 0; i < 50 - 1; i++)
            to_up_stream->send_message(this->chunk_size);
    }
    else
    {
        LOG(INFO) << "Runing in Ring Allreduce";
    }
    do
    {
        message_signal = this->event_siganl.pop();
        switch (message_signal)
        {
        case 1:
            received_data = aggregator->receive_message(0);
            if (is_ring)
                to_up_stream->send_message(this->chunk_size);
            else
            {
                to_down_stream->send_message(this->chunk_size);
            }
            break;
        case 2:
            received_data = to_down_stream->receive_message(0);
            bytes_received += received_data;
            if (is_ring)
            {
                if (reduce_scatter < this->node_size - 1)
                {
                    //LOG(INFO) << "Send to aggregator";
                    aggregator->send_message(2, received_data / sizeof(float));
                    //to_up_stream->send_message(received_data);
                }
                else
                {
                    to_up_stream->send_message(received_data);
                }
            }
            else
            {
                LOG(INFO) << "send to aggregator";
                aggregator->send_message(upper_node_num + 1, received_data / sizeof(float));
            }

            reduce_scatter++;
            reduce_scatter %= (2 * this->node_size - 2);
            break;
        case 3:
            received_data = to_up_stream->receive_message(0);
            bytes_received += received_data;
            if (is_ring)
            {
                to_down_stream->send_message(received_data);
            }
            else
            {
                // PS, launch the next message
                to_up_stream->send_message(received_data);
            }
            break;
        case 4:
            std::cout << "[Forward_engine]: Receive message from data generator" << std::endl;
            break;
        default:
            std::cerr << "[Forward_engine]: Receive unknown message: " << message_signal << std::endl;
            break;
        }
        if (bytes_received * upper_node_num > 10000000000)
        {
            timer.stop();
            LOG(INFO) << "Total recv rate: " << 8 * bytes_received * upper_node_num / 1000.0 / 1000 / 1000 / timer.seconds() << " Gbps";
            timer.start();
            bytes_received = 0;
        }
    } while (true);
}