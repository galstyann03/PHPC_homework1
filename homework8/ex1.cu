#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <cuda_runtime.h>

#define N 10000000
#define PRINT_COUNT 50

__global__ void complement_dna(const char *input, char *output, int count)
{
    int tid = blockIdx.x * blockDim.x + threadIdx.x;

    if (tid < count) {
        char nucleotide = input[tid];

        if (nucleotide == 'A') {
            output[tid] = 'T';
        } else if (nucleotide == 'T') {
            output[tid] = 'A';
        } else if (nucleotide == 'C') {
            output[tid] = 'G';
        } else {
            output[tid] = 'C';
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

void print_first_50(const char *label, const char *sequence)
{
    printf("%s", label);

    for (int i = 0; i < PRINT_COUNT; i++) {
        printf("%c", sequence[i]);
    }

    printf("\n");
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
    size_t bytes = N * sizeof(char);

    char *host_input = (char*)malloc(bytes);
    char *host_output = (char*)malloc(bytes);
    char *device_input = NULL;
    char *device_output = NULL;

    if (!host_input || !host_output) {
        printf("Host memory allocation failed\n");
        free(host_input);
        free(host_output);
        return 1;
    }

    srand((unsigned int)time(NULL));
    generate_dna(host_input, N);

    if (!check_cuda(cudaMalloc((void**)&device_input, bytes), "cudaMalloc device_input")) {
        free(host_input);
        free(host_output);
        return 1;
    }

    if (!check_cuda(cudaMalloc((void**)&device_output, bytes), "cudaMalloc device_output")) {
        cudaFree(device_input);
        free(host_input);
        free(host_output);
        return 1;
    }

    if (!check_cuda(cudaMemcpy(device_input, host_input, bytes, cudaMemcpyHostToDevice), "cudaMemcpy host to device")) {
        cudaFree(device_input);
        cudaFree(device_output);
        free(host_input);
        free(host_output);
        return 1;
    }

    for (int i = 0; i < run_count; i++) {
        int block_size = block_sizes[i];
        int grid_size = (N + block_size - 1) / block_size;
        int total_threads = grid_size * block_size;
        float kernel_time_ms = 0.0f;
        cudaEvent_t start;
        cudaEvent_t stop;

        if (!check_cuda(cudaEventCreate(&start), "cudaEventCreate start")) {
            cudaFree(device_input);
            cudaFree(device_output);
            free(host_input);
            free(host_output);
            return 1;
        }

        if (!check_cuda(cudaEventCreate(&stop), "cudaEventCreate stop")) {
            cudaEventDestroy(start);
            cudaFree(device_input);
            cudaFree(device_output);
            free(host_input);
            free(host_output);
            return 1;
        }

        cudaEventRecord(start);
        complement_dna<<<grid_size, block_size>>>(device_input, device_output, N);
        cudaEventRecord(stop);

        if (!check_cuda(cudaGetLastError(), "Kernel launch")) {
            cudaEventDestroy(start);
            cudaEventDestroy(stop);
            cudaFree(device_input);
            cudaFree(device_output);
            free(host_input);
            free(host_output);
            return 1;
        }

        if (!check_cuda(cudaEventSynchronize(stop), "cudaEventSynchronize")) {
            cudaEventDestroy(start);
            cudaEventDestroy(stop);
            cudaFree(device_input);
            cudaFree(device_output);
            free(host_input);
            free(host_output);
            return 1;
        }

        if (!check_cuda(cudaEventElapsedTime(&kernel_time_ms, start, stop), "cudaEventElapsedTime")) {
            cudaEventDestroy(start);
            cudaEventDestroy(stop);
            cudaFree(device_input);
            cudaFree(device_output);
            free(host_input);
            free(host_output);
            return 1;
        }

        if (!check_cuda(cudaMemcpy(host_output, device_output, bytes, cudaMemcpyDeviceToHost), "cudaMemcpy device to host")) {
            cudaEventDestroy(start);
            cudaEventDestroy(stop);
            cudaFree(device_input);
            cudaFree(device_output);
            free(host_input);
            free(host_output);
            return 1;
        }

        printf("=== Block size %d ===\n", block_size);
        print_first_50("Original sequence     : ", host_input);
        print_first_50("Complemented sequence : ", host_output);
        printf("Total threads launched: %d\n", total_threads);
        printf("Kernel execution time : %.3f ms\n\n", kernel_time_ms);

        cudaEventDestroy(start);
        cudaEventDestroy(stop);
    }

    cudaFree(device_input);
    cudaFree(device_output);
    free(host_input);
    free(host_output);

    return 0;
}
