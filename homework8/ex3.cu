#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <cuda_runtime.h>

#define N 24000000
#define CODON_COUNT (N / 3)
#define CODON_TYPES 64

__device__ int nucleotide_to_index(char nucleotide)
{
    if (nucleotide == 'A') {
        return 0;
    }
    if (nucleotide == 'C') {
        return 1;
    }
    if (nucleotide == 'G') {
        return 2;
    }
    return 3;
}

__global__ void analyze_codons(const char *sequence, int count, unsigned int *codon_counts)
{
    int gid = blockIdx.x * blockDim.x + threadIdx.x;

    if (gid < count) {
        int codon_id = gid / 3;
        int warp_id = threadIdx.x / 32;
        int lane_id = threadIdx.x % 32;

        if ((gid % 3) == 0) {
            int first = nucleotide_to_index(sequence[gid]);
            int second = nucleotide_to_index(sequence[gid + 1]);
            int third = nucleotide_to_index(sequence[gid + 2]);
            int codon_index = first * 16 + second * 4 + third;

            atomicAdd(&codon_counts[codon_index], 1);
        }

        if (gid == 0) {
            printf("Sample thread mapping -> gid: %d, codon_id: %d, warp_id: %d, lane_id: %d\n",
                   gid, codon_id, warp_id, lane_id);
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

void index_to_codon(int index, char codon[4])
{
    const char nucleotides[] = { 'A', 'C', 'G', 'T' };

    codon[0] = nucleotides[index / 16];
    codon[1] = nucleotides[(index / 4) % 4];
    codon[2] = nucleotides[index % 4];
    codon[3] = '\0';
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
    int block_size = 256;
    int grid_size = (N + block_size - 1) / block_size;
    int total_threads = grid_size * block_size;
    int total_warps = (block_size / 32) * grid_size;
    size_t sequence_bytes = N * sizeof(char);
    size_t counts_bytes = CODON_TYPES * sizeof(unsigned int);
    float kernel_time_ms = 0.0f;

    char *host_sequence = (char*)malloc(sequence_bytes);
    unsigned int host_codon_counts[CODON_TYPES] = { 0 };
    char *device_sequence = NULL;
    unsigned int *device_codon_counts = NULL;
    cudaEvent_t start;
    cudaEvent_t stop;

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

    if (!check_cuda(cudaMalloc((void**)&device_codon_counts, counts_bytes), "cudaMalloc device_codon_counts")) {
        cudaFree(device_sequence);
        free(host_sequence);
        return 1;
    }

    if (!check_cuda(cudaMemcpy(device_sequence, host_sequence, sequence_bytes, cudaMemcpyHostToDevice), "cudaMemcpy host to device")) {
        cudaFree(device_sequence);
        cudaFree(device_codon_counts);
        free(host_sequence);
        return 1;
    }

    if (!check_cuda(cudaMemset(device_codon_counts, 0, counts_bytes), "cudaMemset device_codon_counts")) {
        cudaFree(device_sequence);
        cudaFree(device_codon_counts);
        free(host_sequence);
        return 1;
    }

    if (!check_cuda(cudaEventCreate(&start), "cudaEventCreate start")) {
        cudaFree(device_sequence);
        cudaFree(device_codon_counts);
        free(host_sequence);
        return 1;
    }

    if (!check_cuda(cudaEventCreate(&stop), "cudaEventCreate stop")) {
        cudaEventDestroy(start);
        cudaFree(device_sequence);
        cudaFree(device_codon_counts);
        free(host_sequence);
        return 1;
    }

    cudaEventRecord(start);
    analyze_codons<<<grid_size, block_size>>>(device_sequence, N, device_codon_counts);
    cudaEventRecord(stop);

    if (!check_cuda(cudaGetLastError(), "Kernel launch")) {
        cudaEventDestroy(start);
        cudaEventDestroy(stop);
        cudaFree(device_sequence);
        cudaFree(device_codon_counts);
        free(host_sequence);
        return 1;
    }

    if (!check_cuda(cudaEventSynchronize(stop), "cudaEventSynchronize")) {
        cudaEventDestroy(start);
        cudaEventDestroy(stop);
        cudaFree(device_sequence);
        cudaFree(device_codon_counts);
        free(host_sequence);
        return 1;
    }

    if (!check_cuda(cudaEventElapsedTime(&kernel_time_ms, start, stop), "cudaEventElapsedTime")) {
        cudaEventDestroy(start);
        cudaEventDestroy(stop);
        cudaFree(device_sequence);
        cudaFree(device_codon_counts);
        free(host_sequence);
        return 1;
    }

    if (!check_cuda(cudaMemcpy(host_codon_counts, device_codon_counts, counts_bytes, cudaMemcpyDeviceToHost), "cudaMemcpy device to host")) {
        cudaEventDestroy(start);
        cudaEventDestroy(stop);
        cudaFree(device_sequence);
        cudaFree(device_codon_counts);
        free(host_sequence);
        return 1;
    }

    printf("Total number of codons : %d\n", CODON_COUNT);
    printf("Total number of threads: %d\n", total_threads);
    printf("Total number of warps  : %d\n", total_warps);
    printf("Block size             : %d\n", block_size);
    printf("Number of blocks       : %d\n", grid_size);
    printf("Kernel execution time  : %.3f ms\n\n", kernel_time_ms);

    printf("Codon counts\n");
    for (int i = 0; i < CODON_TYPES; i++) {
        char codon[4];
        index_to_codon(i, codon);
        printf("%s: %u\n", codon, host_codon_counts[i]);
    }

    cudaEventDestroy(start);
    cudaEventDestroy(stop);
    cudaFree(device_sequence);
    cudaFree(device_codon_counts);
    free(host_sequence);

    return 0;
}
