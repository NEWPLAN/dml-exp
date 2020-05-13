#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>

#define uint32_t unsigned long

int main(int argc, char **argv)
{
    uint32_t mem_size = 1024 * 1024 * 1024;
    mem_size *= 2;
    char *src_buf = (char *)malloc(mem_size);
    char *dst_buf = (char *)malloc(mem_size);
    if (src_buf == NULL || dst_buf == NULL)
    {
        printf("Error of malloc memory\n");
        exit(-1);
    }
    printf("Size_t: %lu\n", sizeof(size_t));
    size_t allocate_mem = 1000 * 1000 * 1000;
    allocate_mem *= 40;
    void *allocate_buf = 0;
    int ret = posix_memalign((void **)&allocate_buf, sysconf(_SC_PAGESIZE), allocate_mem);
    if (ret)
    {
        printf("Error of malloc mem\n");
        exit(0);
    }
    printf("Malloc mem size: %lu bytes\n", allocate_mem);

    for (uint32_t copy_size = 1000; copy_size < 1024 * 1024 * 500; copy_size *= 10)
    {
        struct timeval start, stop;
        gettimeofday(&start, NULL);
        int try_time = 0;
        for (try_time = 0; try_time < 10000; try_time++)
        {
            uint32_t pos = rand() % (mem_size - copy_size);
            memcpy(dst_buf + pos, src_buf + pos, copy_size);
        }
        gettimeofday(&stop, NULL);
        float time_cost = (stop.tv_usec - start.tv_usec) / 1000000.0 + stop.tv_sec - start.tv_sec;

        printf("Time cost of copy %lu bytes in %d round: %fs, with each: %f ms\n",
               copy_size, try_time, time_cost, time_cost * 1000 / try_time);
    }
    free(allocate_buf);

    return 0;
}