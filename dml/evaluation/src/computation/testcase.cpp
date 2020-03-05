#include <cuda_runtime_api.h>
#include <device_launch_parameters.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#include "tensor_merge.h"
static const int M = 32 * 32 * 10;
static const int N = 32 * 32 * 10;

int cuda_test_main(void)
{
    clock_t start, end;

    // Size of vectors
    int num_data = M * N;

    // Host input vectors
    float *h_a;
    float *h_b;
    //Host output vector
    float *h_c;

    // Device input vectors
    float *d_a;
    float *d_b;
    //Device output vector
    float *d_c;

    // Size, in bytes, of each vector
    size_t bytes = num_data * sizeof(float);

    // Allocate memory for each vector on host
    h_a = (float *)malloc(bytes);
    h_b = (float *)malloc(bytes);
    h_c = (float *)malloc(bytes);

    // Allocate memory for each vector on GPU
    cudaMalloc((void **)(&d_a), bytes);
    cudaMalloc((void **)(&d_b), bytes);
    cudaMalloc((void **)(&d_c), bytes);

    // Initialize vectors on host
    for (int i = 0; i < num_data; i++)
    {
        h_a[i] = 1.1;
        h_b[i] = 2.4;
    }

    { //warmup
        for (int num = 0; num < 100; num++)
        {
            //API_add((int**)a_d, (int**)b_d, (int**)c_d,M,N);
        }
        cudaDeviceSynchronize();
    }
    start = clock(); //开始计时

    // Copy host vectors to device
    cudaMemcpy(d_a, h_a, bytes, cudaMemcpyHostToDevice);
    cudaMemcpy(d_b, h_b, bytes, cudaMemcpyHostToDevice);

    cudaDeviceSynchronize();
    start = clock(); //开始计时
    // dim3 DimGrid(1, 1);
    // dim3 DimBlock(32, 32);
    // add<<<DimGrid, DimBlock>>>(a_d, b_d, c_d,M,N);
    {
        API_add_v2(d_a, d_b, d_c, num_data);
        cudaError_t error = cudaGetLastError();
        if (cudaSuccess != error)
        {
            printf("CUDA error: %s\n", cudaGetErrorString(error));
        }
    }
    cudaDeviceSynchronize();
    end = clock(); //计时结束
    cudaMemcpy(h_c, d_c, bytes, cudaMemcpyDeviceToHost);
    //end = clock(); //计时结束
    if (M * N <= 1024 * 2)
        for (int i = 0; i < M; i++)
        {
            for (int j = 0; j < N; j++)
                printf("%2.2f ", h_c[i * M + j]);
            printf("\n");
        }
    float time1 = (float)(end - start) / CLOCKS_PER_SEC;
    printf("执行时间为：%f s\n", time1);

    // Release device memory
    cudaFree(d_a);
    cudaFree(d_b);
    cudaFree(d_c);

    // Release host memory
    free(h_a);
    free(h_b);
    free(h_c);

    return 0;
}