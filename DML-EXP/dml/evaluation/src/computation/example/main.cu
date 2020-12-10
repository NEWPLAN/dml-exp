#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
 
// CUDA kernel. Each thread takes care of one element of c
__global__ void vecAdd(double *a, double *b, double *c, int n)
{
    // Get our global thread ID
    int id = blockIdx.x*blockDim.x+threadIdx.x;
 
    // Make sure we do not go out of bounds
    if (id < n)
        c[id] = a[id] + b[id];
}

// CUDA kernel. Each thread takes care of one element of c
__global__ void vecAdd3(double *a, double *b, double *c,double *d,int n)
{
    // Get our global thread ID
    int id = blockIdx.x*blockDim.x+threadIdx.x;
 
    // Make sure we do not go out of bounds
    if (id < n)
        d[id] = a[id] + b[id]+c[id];
}

// CUDA kernel. Each thread takes care of one element of c
__global__ void vecAdd4(double *a, double *b, double *c,double *d,double *e,int n)
{
    // Get our global thread ID
    int id = blockIdx.x*blockDim.x+threadIdx.x;
 
    // Make sure we do not go out of bounds
    if (id < n)
        e[id] = a[id] + b[id]+c[id]+d[id];
}

// CUDA kernel. Each thread takes care of one element of c
__global__ void vecAdd5(double *a, double *b, double *c,double *d,double *e,double *f,int n)
{
    // Get our global thread ID
    int id = blockIdx.x*blockDim.x+threadIdx.x;
 
    // Make sure we do not go out of bounds
    if (id < n)
        f[id] = a[id] + b[id]+c[id]+d[id]+e[id];
}
 
int main( int argc, char* argv[] )
{
    // Size of vectors
    int n = 100000;

    int tensor_num=0;
    int tensor_size=0;

    clock_t start1,start2, end2,end1;
 
    // Host input vectors
    double *h_a;
    double *h_b;
    //Host output vector
    double *h_c;
    double *h_d;
    double *h_e;
    double *h_f;
 
    // Device input vectors
    double *d_a;
    double *d_b;
    //Device output vector
    double *d_c;
    double *d_d;
    double *d_e;
    double *d_f;
    {
       tensor_num=atoi(argv[1]);
       tensor_size=atoi(argv[2]);
       n=tensor_size;
       //printf("Tensor num: %d, tensor_size: %d\n",tensor_num,tensor_size);
    }
 
    // Size, in bytes, of each vector
    size_t bytes = n*sizeof(double);
 
    // Allocate memory for each vector on host
    h_a = (double*)malloc(bytes);
    h_b = (double*)malloc(bytes);
    h_c = (double*)malloc(bytes);
    h_d = (double*)malloc(bytes);
    h_e = (double*)malloc(bytes);
    h_f = (double*)malloc(bytes);
    
 
    // Allocate memory for each vector on GPU
    cudaMalloc(&d_a, bytes);
    cudaMalloc(&d_b, bytes);
    cudaMalloc(&d_c, bytes);
    cudaMalloc(&d_d, bytes);
    cudaMalloc(&d_e, bytes);
    cudaMalloc(&d_f, bytes);
 
    int i;
    // Initialize vectors on host
    for( i = 0; i < n; i++ ) {
        h_a[i] = sin(i)*sin(i);
        h_b[i] = cos(i)*cos(i);
    }  

    start1=clock();
 
    // Copy host vectors to device
    cudaMemcpy( d_a, h_a, bytes, cudaMemcpyHostToDevice);
    cudaMemcpy( d_b, h_b, bytes, cudaMemcpyHostToDevice);
    if(tensor_num>=3)
        cudaMemcpy( d_c, h_c, bytes, cudaMemcpyHostToDevice);
    if(tensor_num>=4)
        cudaMemcpy( d_d, h_d, bytes, cudaMemcpyHostToDevice);
    if(tensor_num>=5)
        cudaMemcpy( d_e, h_e, bytes, cudaMemcpyHostToDevice);
 
    int blockSize, gridSize;
 
    // Number of threads in each thread block
    blockSize = 1024;
 
    // Number of thread blocks in grid
    gridSize = (int)ceil((float)n/blockSize);
cudaDeviceSynchronize();
    start2=clock();
 
    // Execute the kernel
    if(tensor_num==2)
        vecAdd<<<gridSize, blockSize>>>(d_a, d_b, d_c, n);
    if(tensor_num==3)
        vecAdd3<<<gridSize, blockSize>>>(d_a, d_b, d_c,d_d, n);
    if(tensor_num==4)
        vecAdd4<<<gridSize, blockSize>>>(d_a, d_b, d_c,d_d,d_e, n);
    if(tensor_num==5)
        vecAdd5<<<gridSize, blockSize>>>(d_a, d_b, d_c,d_d,d_e,d_f, n);
    cudaDeviceSynchronize();
    end2=clock();
    // Copy array back to host
    cudaMemcpy( h_c, d_c, bytes, cudaMemcpyDeviceToHost );
    end1=clock();
    // Sum up vector c and print result divided by n, this should equal 1 within error
    double sum = 0;
    //for(i=0; i<n; i++)
    //    sum += h_c[i];
    //printf("final result: %f\n", sum/n);

    float time1 = (float)(end1 - start1) / CLOCKS_PER_SEC;
    float time2 = (float)(end2 - start2) / CLOCKS_PER_SEC;
    printf("[%d, %d]: %f ms, computing: %f ms\n", tensor_num,tensor_size, time1*1000, time2*1000);
 
    // Release device memory
    cudaFree(d_a);
    cudaFree(d_b);
    cudaFree(d_c);
    cudaFree(d_e);
    cudaFree(d_f);
 
    // Release host memory
    free(h_a);
    free(h_b);
    free(h_c);
    free(h_d);
    free(h_e);
    free(h_f);
 
    return 0;
}