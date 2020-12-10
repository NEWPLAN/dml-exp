#include "thread_pool.hpp"
#include <iostream>
#include "ATimer.h"
#include "utils.h"

#include "test.h"

int main(int argc, char const *argv[])
{
	if (argc != 2)
	{
		std::cout << "Usage: ./exe chunk_size" << std::endl;
		return -1;
	}
	int chunk_size = atoi(argv[1]);
	std::cout << "Chunk size: " << chunk_size << std::endl;
	Benchmark *test_unit = new Benchmark(chunk_size);
	test_unit->setup_network("12.12.12.111", 1234);
	//test_unit->setup_computation();
	test_unit->setup_aggregator();
	test_unit->setup_dataGenetator();
	test_unit->run();

	test_unit->forward_engine_start();

	test_unit->wait_for_done();

	return 0;
}