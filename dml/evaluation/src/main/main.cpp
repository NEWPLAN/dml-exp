#include "thread_pool.hpp"
#include <iostream>
#include "ATimer.h"
#include "utils.h"

#include "test.h"

// #include <unistd.h>
// #include <sys/syscall.h>

// std::uint32_t lwp_id()
// {
// #if defined(APPLE)
// 	return static_cast<std::uint32_t>(std::this_thread::get_id());
// #else
// 	return static_cast<std::uint32_t>(syscall(SYS_gettid));
// #endif
// }

void tensor_merge(float *result, float **data, int num_tensor, int start, int stop)
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

void do_test(newplan::ThreadPool &context, float *result, float **data, int num_tensor, int size)
{
	for (int nums = 0; nums < size; nums += size / 20)
	{
		int end_posi = nums + size / 20;
		context.runTask([result, data, num_tensor, nums, end_posi]() {
			tensor_merge(result, (float **)data, num_tensor, nums, end_posi);
		});
	}
	context.waitWorkComplete();
	return;
}
void do_init(newplan::ThreadPool &context, int *data, int data_size)
{
	for (size_t i = 0; i < 20; i++)
	{
		context.runTask([data, data_size]() {for (int i=0;i<data_size;i++)
		{
			data[i]=(data[i]%191)*(data[i]%99983);
		} });
	}
	context.waitWorkComplete();
	return;
}
#include "tensor_merge.h"

#define KB 1000
int main(int argc, char const *argv[])
{
	std::string ip_prefix("12.12.12.");
	std::cout << "node ID: " << ip_parser(ip_prefix.c_str(), 9) << std::endl;
	Solution *solution = new Solution();
	solution->setup_network("12.12.12.111", 1234);
	solution->setup_computation();
	solution->run();

	solution->wait_for_done();
}