#include "thread_pool.hpp"
#include <iostream>
#include "ATimer.h"
#include "utils.h"

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

	float *result;
	float *data[5];

	int size = 1000 * KB;
	int num_tensor = 2;
	{
		std::cout << "tensor nums, and size to merge: ";
		std::cin >> num_tensor >> size;
	}
	{ // init for evaluation
		result = (float *)malloc(size * sizeof(float) + 100);
		data[0] = (float *)malloc(size * sizeof(float) + 100);
		data[1] = (float *)malloc(size * sizeof(float) + 100);
		data[2] = (float *)malloc(size * sizeof(float) + 100);
		data[3] = (float *)malloc(size * sizeof(float) + 100);
		data[4] = (float *)malloc(size * sizeof(float) + 100);
		for (int i = 0; i < size + 100; i++)
		{
			result[i] = 0;
			data[0][i] = 1.1;
			data[1][i] = 1.2;
			data[2][i] = 1.3;
			data[3][i] = 1.4;
			data[4][i] = 1;
		}
	}
	int *ramdom = new int[1024 * 1024 * 100];
	int random_size = 1024 * 1024 * 100;

	{
		{
			cuda_test_main();
		}
	}

	newplan::ThreadPool tp(10);
	std::mutex m;
	for (int data_size = 1000; data_size <= size; data_size *= 10)
	{
		for (int ten_num = 2; ten_num <= num_tensor; ten_num++)
		{
			for (int loops = 0; loops < 5; loops++)
			{
				Timer t1;
				t1.start();
				{
					do_test(tp, result, data, ten_num, data_size);
				}
				t1.stop();
				std::cout << "num tensor: " << ten_num << ", size: " << data_size / 1000 << " KB, Time cost: " << t1.milliseconds() << " ms" << std::endl;

				{
					do_init(tp, ramdom, random_size);
				}
			}
		}
	}

	for (int i = 0; i < size; i++)
	{
		if (result[i] != 6)
		{
			std::cerr << "error in posi: " << i << ", " << result[i] << std::endl;
			return -1;
		}
	}

	std::cout << "Done!" << std::endl;
	return 0;
}