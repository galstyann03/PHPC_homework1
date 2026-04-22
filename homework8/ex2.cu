#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <cuda_runtime.h>

#define N 20000000

__global__ void count_bases(const char *sequence, int count, unsigned int *count_a, unsigned int *count_c, unsigned int *count_g, unsigned int *count_t)
{
    int tid = blockIdx.x * blockDim.x + threadIdx.x;

    if (tid < count) {
        char nucleotide = sequence[tid];

        if (nucleotide == 'A') {
            atomicAdd(count_a, 1);
        } else if (nucleotide == 'C') {
            atomicAdd(count_c, 1);
        } else if (nucleotide == 'G') {
            atomicAdd(count_g, 1);
        } else {
            atomicAdd(count_t, 1);
        }
    }
}

void generate_dna(char *sequence, int count)
{
    const char nucleotides[] = { 'A', 'C', 'G', 'T' };

    for (int i = 0; i < count; i++) {
        sequence[i] = nucleotides[rand() % 4];
    }
}

int check_cuda(cudaError_t err, const char *step)
{
    if (err != cudaSuccess) {
        printf("%s failed: %s\n", step, cudaGetErrorString(err));
        return 0;
    }

    return 1;
}

int main(void)
{
    int block_sizes[] = { 64, 128, 256, 512 };
    int run_count = (int)(sizeof(block_sizes) / sizeof(block_sizes[0]));
    size_t sequence_bytes = N * sizeof(char);
    size_t counter_bytes = sizeof(unsigned int);

    char *host_sequence = (char*)malloc(sequence_bytes);
    char *device_sequence = NULL;
    unsigned int *device_count_a = NULL;
    unsigned int *device_count_c = NULL;
    unsigned int *device_count_g = NULL;
    unsigned int *device_count_t = NULL;

    if (!host_sequence) {
        printf("Host memory allocation failed\n");
        return 1;
    }

    srand((unsigned int)time(NULL));
    generate_dna(host_sequence, N);

    if (!check_cuda(cudaMalloc((void**)&device_sequence, sequence_bytes), "cudaMalloc device_sequence")) {
        free(host_sequence);
        return 1;
    }

    if (!check_cuda(cudaMalloc((void**)&device_count_a, counter_bytes), "cudaMalloc device_count_a")) {
        cudaFree(device_sequence);
        free(host_sequence);
        return 1;
    }

    if (!check_cuda(cudaMalloc((void**)&device_count_c, counter_bytes), "cudaMalloc device_count_c")) {
        cudaFree(device_sequence);
        cudaFree(device_count_a);
        free(host_sequence);
        return 1;
    }

    if (!check_cuda(cudaMalloc((void**)&device_count_g, counter_bytes), "cudaMalloc device_count_g")) {
        cudaFree(device_sequence);
        cudaFree(device_count_a);
        cudaFree(device_count_c);
        free(host_sequence);
        return 1;
    }

    if (!check_cuda(cudaMalloc((void**)&device_count_t, counter_bytes), "cudaMalloc device_count_t")) {
        cudaFree(device_sequence);
        cudaFree(device_count_a);
        cudaFree(device_count_c);
        cudaFree(device_count_g);
        free(host_sequence);
        return 1;
    }

    if (!check_cuda(cudaMemcpy(device_sequence, host_sequence, sequence_bytes, cudaMemcpyHostToDevice), "cudaMemcpy host to device")) {
        cudaFree(device_sequence);
        cudaFree(device_count_a);
        cudaFree(device_count_c);
        cudaFree(device_count_g);
        cudaFree(device_count_t);
        free(host_sequence);
        return 1;
    }

    for (int i = 0; i < run_count; i++) {
        int block_size = block_sizes[i];
        int grid_size = (N + block_size - 1) / block_size;
        unsigned int count_a = 0;
        unsigned int count_c = 0;
        unsigned int count_g = 0;
        unsigned int count_t = 0;
        unsigned int total = 0;
        float kernel_time_ms = 0.0f;
        cudaEvent_t start;
        cudaEvent_t stop;

        if (!check_cuda(cudaMemset(device_count_a, 0, counter_bytes), "cudaMemset device_count_a")) {
            cudaFree(device_sequence);
            cudaFree(device_count_a);
            cudaFree(device_count_c);
            cudaFree(device_count_g);
            cudaFree(device_count_t);
            free(host_sequence);
            return 1;
        }

        if (!check_cuda(cudaMemset(device_count_c, 0, counter_bytes), "cudaMemset device_count_c")) {
            cudaFree(device_sequence);
            cudaFree(device_count_a);
            cudaFree(device_count_c);
            cudaFree(device_count_g);
            cudaFree(device_count_t);
            free(host_sequence);
            return 1;
        }

        if (!check_cuda(cudaMemset(device_count_g, 0, counter_bytes), "cudaMemset device_count_g")) {
            cudaFree(device_sequence);
            cudaFree(device_count_a);
            cudaFree(device_count_c);
            cudaFree(device_count_g);
            cudaFree(device_count_t);
            free(host_sequence);
            return 1;
        }

        if (!check_cuda(cudaMemset(device_count_t, 0, counter_bytes), "cudaMemset device_count_t")) {
            cudaFree(device_sequence);
            cudaFree(device_count_a);
            cudaFree(device_count_c);
            cudaFree(device_count_g);
            cudaFree(device_count_t);
            free(host_sequence);
            return 1;
        }

        if (!check_cuda(cudaEventCreate(&start), "cudaEventCreate start")) {
            cudaFree(device_sequence);
            cudaFree(device_count_a);
            cudaFree(device_count_c);
            cudaFree(device_count_g);
            cudaFree(device_count_t);
            free(host_sequence);
            return 1;
        }

        if (!check_cuda(cudaEventCreate(&stop), "cudaEventCreate stop")) {
            cudaEventDestroy(start);
            cudaFree(device_sequence);
            cudaFree(device_count_a);
            cudaFree(device_count_c);
            cudaFree(device_count_g);
            cudaFree(device_count_t);
            free(host_sequence);
            return 1;
        }

        cudaEventRecord(start);
        count_bases<<<grid_size, block_size>>>(device_sequence, N, device_count_a, device_count_c, device_count_g, device_count_t);
        cudaEventRecord(stop);

        if (!check_cuda(cudaGetLastError(), "Kernel launch")) {
            cudaEventDestroy(start);
            cudaEventDestroy(stop);
            cudaFree(device_sequence);
            cudaFree(device_count_a);
            cudaFree(device_count_c);
            cudaFree(device_count_g);
            cudaFree(device_count_t);
            free(host_sequence);
            return 1;
        }

        if (!check_cuda(cudaEventSynchronize(stop), "cudaEventSynchronize")) {
            cudaEventDestroy(start);
            cudaEventDestroy(stop);
            cudaFree(device_sequence);
            cudaFree(device_count_a);
            cudaFree(device_count_c);
            cudaFree(device_count_g);
            cudaFree(device_count_t);
            free(host_sequence);
            return 1;
        }

        if (!check_cuda(cudaEventElapsedTime(&kernel_time_ms, start, stop), "cudaEventElapsedTime")) {
            cudaEventDestroy(start);
            cudaEventDestroy(stop);
            cudaFree(device_sequence);
            cudaFree(device_count_a);
            cudaFree(device_count_c);
            cudaFree(device_count_g);
            cudaFree(device_count_t);
            free(host_sequence);
            return 1;
        }

        if (!check_cuda(cudaMemcpy(&count_a, device_count_a, counter_bytes, cudaMemcpyDeviceToHost), "cudaMemcpy count_a")) {
            cudaEventDestroy(start);
            cudaEventDestroy(stop);
            cudaFree(device_sequence);
            cudaFree(device_count_a);
            cudaFree(device_count_c);
            cudaFree(device_count_g);
            cudaFree(device_count_t);
            free(host_sequence);
            return 1;
        }

        if (!check_cuda(cudaMemcpy(&count_c, device_count_c, counter_bytes, cudaMemcpyDeviceToHost), "cudaMemcpy count_c")) {
            cudaEventDestroy(start);
            cudaEventDestroy(stop);
            cudaFree(device_sequence);
            cudaFree(device_count_a);
            cudaFree(device_count_c);
            cudaFree(device_count_g);
            cudaFree(device_count_t);
            free(host_sequence);
            return 1;
        }

        if (!check_cuda(cudaMemcpy(&count_g, device_count_g, counter_bytes, cudaMemcpyDeviceToHost), "cudaMemcpy count_g")) {
            cudaEventDestroy(start);
            cudaEventDestroy(stop);
            cudaFree(device_sequence);
            cudaFree(device_count_a);
            cudaFree(device_count_c);
            cudaFree(device_count_g);
            cudaFree(device_count_t);
            free(host_sequence);
            return 1;
        }

        if (!check_cuda(cudaMemcpy(&count_t, device_count_t, counter_bytes, cudaMemcpyDeviceToHost), "cudaMemcpy count_t")) {
            cudaEventDestroy(start);
            cudaEventDestroy(stop);
            cudaFree(device_sequence);
            cudaFree(device_count_a);
            cudaFree(device_count_c);
            cudaFree(device_count_g);
            cudaFree(device_count_t);
            free(host_sequence);
            return 1;
        }

        total = count_a + count_c + count_g + count_t;

        printf("=== Block size %d ===\n", block_size);
        printf("Number of blocks     : %d\n", grid_size);
        printf("Count of A           : %u\n", count_a);
        printf("Count of C           : %u\n", count_c);
        printf("Count of G           : %u\n", count_g);
        printf("Count of T           : %u\n", count_t);
        printf("Total sum of counts  : %u\n", total);
        printf("Kernel execution time: %.3f ms\n\n", kernel_time_ms);

        cudaEventDestroy(start);
        cudaEventDestroy(stop);
    }

    cudaFree(device_sequence);
    cudaFree(device_count_a);
    cudaFree(device_count_c);
    cudaFree(device_count_g);
    cudaFree(device_count_t);
    free(host_sequence);

    return 0;
}
