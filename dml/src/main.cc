#include <iostream>
#include <ctime>
#include <unistd.h>
#include <chrono>
#include <thread>
using namespace std;

#include "ATimer.h"

void print_hello(void)
{
    //sleep(1.5);
    std::this_thread::sleep_for(chrono::milliseconds(50));
    //for(int i = 0; i <1000;i++);
}

void tensor_merge(float *result, float **data, int num_tensor, int size)
{
    for (int i = 0; i < size; i++)
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
#define KB 1000
int main(int argc, char **argv)
{
    float *result;
    float *data[5];

    int size = 1000 * KB;
    int num_tensor = 2;
    if (argc >= 2)
    {
        size = atoi(argv[1]) * KB;
        num_tensor = atoi(argv[2]);
    }
    std::cout << "Tensor Size: " << size / KB << " KB, Tensor Number: " << num_tensor << std::endl;

    result = (float *)malloc(size * sizeof(float));
    data[0] = (float *)malloc(size * sizeof(float));
    data[1] = (float *)malloc(size * sizeof(float));
    data[2] = (float *)malloc(size * sizeof(float));
    data[3] = (float *)malloc(size * sizeof(float));
    data[4] = (float *)malloc(size * sizeof(float));
    int data_size = KB;
    Timer t1;
    // {
    //     while (data_size <= size)
    //     {
    //         for (int num_tensor = 2; num_tensor <= 5; num_tensor++)
    //         {
    t1.start();
    tensor_merge(result, data, num_tensor, size);
    t1.stop();

    cout << size / 1000 << " KB, " << num_tensor << " tensors, using Atimer: " << t1.milliseconds() << " ms" << endl;
    //     }
    //     data_size *= 10;
    // }
    //}

    //std::cout<<"time use:"<<1000*(time_end-time_start)/(double)CLOCKS_PER_SEC<<"ms"<<std::endl;
    return 0;
}