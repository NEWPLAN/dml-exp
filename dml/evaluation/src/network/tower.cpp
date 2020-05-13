#include "tower.h"
#include <iostream>
#include "../utils/logging.h"

Tower::Tower(TYPE type)
{
    this->type_ = type;
    this->server_guard = new TCPServer();
    LOG(INFO) << "Building tower ";
}
Tower::~Tower()
{
    LOG(INFO) << "Destroy tower";
}
void Tower::send_message(int size)
{
    if (this->type_ == UPPER_STREAM)
    {
        if (this->client_group.size() == 0)
        {
            std::cout << "Error of upperstream, can not be zero connection" << std::endl;
            exit(-1);
        }
        for (auto &upper : this->client_group)
        {
            upper->send_data(size);
        }
    }
    else
    {
        if (this->server_group.size() == 0)
        {
            std::cout << "Error of downstream, can not be zero connection" << std::endl;
            exit(-1);
        }
        for (auto &upper : this->server_group)
        {
            upper->send_data(size);
        }
    }
}
int Tower::receive_message(int size)
{
    if (recv_queue == nullptr)
    {
        this->recv_queue = this->server_guard->get_recv_channel();
    }
    return this->recv_queue->pop();
}

void Tower::build_network_system(std::string ip, int port)
{
    this->server_ip = ip;
    this->listen_port = port;
    { //all tower should listen and wait for connections;
        this->server_guard->setup("0.0.0.0", port);
        if (this->type_ == DOWN_STREAM)
        {

            LOG(INFO) << "[Down]: On listening: " << port;
        }
        else
        {
            LOG(INFO) << "[Upper]: On listening: " << port;
        }
    }
    if (this->type_ == UPPER_STREAM) //upperstream-->downstream
    {
        TCPClient *tmp_client = new TCPClient();
        LOG(INFO) << "[upperstream-->downstream]: On connecting: " << port + 1;
        tmp_client->setup(ip, port + 1);
        this->client_group.push_back(tmp_client);
    }
    {
        this->server_guard->start_service();
    }
}

void Tower::build_network_system(std::vector<std::string> ip, int port)
{
    this->ip_groups = ip;
    this->listen_port = port;
    { //all tower should listen and wait for connections;
        this->server_guard->setup("0.0.0.0", port);
        if (this->type_ == DOWN_STREAM)
        {
            std::cout << "[Down]: On listening: " << port << std::endl;
        }
        else
        {
            std::cout << "[Upper]: On listening: " << port << std::endl;
        }
    }

    {
        this->server_guard->start_service(ip.size());
    }
}

void Tower::start_service()
{
    {
        if (this->type_ == UPPER_STREAM) //connect to upper stream
        {
            for (auto &upper_ip : this->ip_groups)
            {
                TCPClient *tmp_client = new TCPClient();
                LOG(INFO) << "[to upperstream]: On connecting: "
                          << upper_ip << ":" << this->listen_port + 1;
                tmp_client->setup(upper_ip, this->listen_port + 1);
                this->client_group.push_back(tmp_client);
            }
            for (auto &upper : this->client_group)
            {
                upper->start_service();
            }
        }

        if (this->type_ == DOWN_STREAM)
        {
            for (auto &down_ip : this->ip_groups)
            {
                TCPClient *tmp_client = new TCPClient();
                LOG(INFO) << "[to downstream]:On connecting: "
                          << down_ip << ":" << this->listen_port - 1;

                tmp_client->setup(down_ip, this->listen_port - 1);
                tmp_client->start_service();
                this->server_group.push_back(tmp_client);
            }
        }
    }
}