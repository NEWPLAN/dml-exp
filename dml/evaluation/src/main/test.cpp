#include "test.h"
#include <iostream>

Solution::Solution()
{
    std::cout << "Building test case" << std::endl;
    server = new TCPServer();
    client = new TCPClient();
    compute_engine = new ComputeEngine();
    std::cout << "Test case has been built" << std::endl;
}
Solution::~Solution()
{
    std::cout << "Destroy solution" << std::endl;
}

void Solution::run()
{

    server->start_service();
    std::cout << "Server has been serving" << std::endl;
    client->start_service();
    std::cout << "Client has been serving" << std::endl;
    compute_engine->run();
    std::cout << "ComputeEngine has been serving" << std::endl;

    std::cout << "All modules has been launched" << std::endl;
}

void Solution::setup_network(std::string ip, short port)
{
    std::cout << "IP: " << ip << ", port: " << port << std::endl;
    server->setup("0.0.0.0", port);
    client->setup(ip, port);
}
void Solution::setup_computation()
{
    compute_engine->setup(100 * 1000 * 1000, 5, 10);
}

void Solution::wait_for_done()
{
    std::cout << "Main thread is running in background" << std::endl;
    do
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    } while (1);
}