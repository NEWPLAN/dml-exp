#include <cuda_runtime_api.h>
#include <device_launch_parameters.h>
#include <stdio.h>
#include <time.h>



// CUDA kernel. Each thread takes care of one element of c
__global__ void vecAdd(float *a, float *b, float *c, int n)
{
    // Get our global thread ID
    int id = blockIdx.x*blockDim.x+threadIdx.x;
 
    // Make sure we do not go out of bounds
    if (id < n)
        c[id] = a[id] + b[id];
}
#include <math.h>
void API_add_v2(float *a, float *b, float *c, int data_num)
{
    printf("In function: data nums = %d\n",data_num);

    // Number of threads in each thread block
    int blockSize = 1024;
 
    // Number of thread blocks in grid
    int gridSize = (int)ceil((float)data_num/blockSize);
    vecAdd<<<gridSize, blockSize>>>(a, b, c, data_num);
}