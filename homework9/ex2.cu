#include <stdio.h>
#include <string.h>
#include <cuda_runtime.h>

#define MAX_RNA 10001
#define MAX_PROTEIN 3334

__device__ int getValue(char c)
{
    if (c == 'U') return 0;
    if (c == 'C') return 1;
    if (c == 'A') return 2;
    if (c == 'G') return 3;

    return 0;
}

__device__ char getAmino(char a, char b, char c)
{
    char table[] = "FFLLSSSSYY**CC*WLLLLPPPPHHQQRRRRIIIMTTTTNNKKSSRRVVVVAAAADDEEGGGG";
    int index = getValue(a) * 16 + getValue(b) * 4 + getValue(c);

    return table[index];
}

__global__ void translateRNA(const char *rna, char *protein, int codonCount)
{
    int tid = blockIdx.x * blockDim.x + threadIdx.x;

    if (tid < codonCount) {
        int pos = tid * 3;
        protein[tid] = getAmino(rna[pos], rna[pos + 1], rna[pos + 2]);
    }
}

int checkCuda(cudaError_t err, const char *step)
{
    if (err != cudaSuccess) {
        printf("%s failed: %s\n", step, cudaGetErrorString(err));
        return 0;
    }

    return 1;
}

int main()
{
    char hostRNA[MAX_RNA];
    char hostProtein[MAX_PROTEIN];
    char *deviceRNA = NULL;
    char *deviceProtein = NULL;
    int blockSize = 256;

    scanf("%10000s", hostRNA);

    int rnaLength = (int)strlen(hostRNA);
    int codonCount = rnaLength / 3;
    int gridSize = (codonCount + blockSize - 1) / blockSize;
    size_t rnaBytes = rnaLength * sizeof(char);
    size_t proteinBytes = codonCount * sizeof(char);

    if (!checkCuda(cudaMalloc((void**)&deviceRNA, rnaBytes), "cudaMalloc deviceRNA")) {
        return 1;
    }

    if (!checkCuda(cudaMalloc((void**)&deviceProtein, proteinBytes), "cudaMalloc deviceProtein")) {
        cudaFree(deviceRNA);
        return 1;
    }

    if (!checkCuda(cudaMemcpy(deviceRNA, hostRNA, rnaBytes, cudaMemcpyHostToDevice), "cudaMemcpy host to device")) {
        cudaFree(deviceRNA);
        cudaFree(deviceProtein);
        return 1;
    }

    translateRNA<<<gridSize, blockSize>>>(deviceRNA, deviceProtein, codonCount);

    if (!checkCuda(cudaGetLastError(), "Kernel launch")) {
        cudaFree(deviceRNA);
        cudaFree(deviceProtein);
        return 1;
    }

    if (!checkCuda(cudaDeviceSynchronize(), "cudaDeviceSynchronize")) {
        cudaFree(deviceRNA);
        cudaFree(deviceProtein);
        return 1;
    }

    if (!checkCuda(cudaMemcpy(hostProtein, deviceProtein, proteinBytes, cudaMemcpyDeviceToHost), "cudaMemcpy device to host")) {
        cudaFree(deviceRNA);
        cudaFree(deviceProtein);
        return 1;
    }

    for (int i = 0; i < codonCount; i++) {
        if (hostProtein[i] == '*') {
            hostProtein[i] = '\0';
            break;
        }

        if (i == codonCount - 1) {
            hostProtein[codonCount] = '\0';
        }
    }

    printf("%s\n", hostProtein);

    cudaFree(deviceRNA);
    cudaFree(deviceProtein);

    return 0;
}
