#include <stdio.h>
#include <string.h>
#include <cuda_runtime.h>

#define MAX_SIZE 1001

__global__ void compareStrings(const char *s, const char *t, int *different, int count)
{
    int tid = blockIdx.x * blockDim.x + threadIdx.x;

    if (tid < count) {
        if (s[tid] != t[tid]) {
            different[tid] = 1;
        } else {
            different[tid] = 0;
        }
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
    char hostS[MAX_SIZE];
    char hostT[MAX_SIZE];
    int hostDifferent[MAX_SIZE];
    char *deviceS = NULL;
    char *deviceT = NULL;
    int *deviceDifferent = NULL;
    int blockSize = 256;
    int distance = 0;

    scanf("%1000s", hostS);
    scanf("%1000s", hostT);

    int count = (int)strlen(hostS);
    int gridSize = (count + blockSize - 1) / blockSize;
    size_t stringBytes = count * sizeof(char);
    size_t intBytes = count * sizeof(int);

    if (!checkCuda(cudaMalloc((void**)&deviceS, stringBytes), "cudaMalloc deviceS")) {
        return 1;
    }

    if (!checkCuda(cudaMalloc((void**)&deviceT, stringBytes), "cudaMalloc deviceT")) {
        cudaFree(deviceS);
        return 1;
    }

    if (!checkCuda(cudaMalloc((void**)&deviceDifferent, intBytes), "cudaMalloc deviceDifferent")) {
        cudaFree(deviceS);
        cudaFree(deviceT);
        return 1;
    }

    if (!checkCuda(cudaMemcpy(deviceS, hostS, stringBytes, cudaMemcpyHostToDevice), "cudaMemcpy s to device")) {
        cudaFree(deviceS);
        cudaFree(deviceT);
        cudaFree(deviceDifferent);
        return 1;
    }

    if (!checkCuda(cudaMemcpy(deviceT, hostT, stringBytes, cudaMemcpyHostToDevice), "cudaMemcpy t to device")) {
        cudaFree(deviceS);
        cudaFree(deviceT);
        cudaFree(deviceDifferent);
        return 1;
    }

    compareStrings<<<gridSize, blockSize>>>(deviceS, deviceT, deviceDifferent, count);

    if (!checkCuda(cudaGetLastError(), "Kernel launch")) {
        cudaFree(deviceS);
        cudaFree(deviceT);
        cudaFree(deviceDifferent);
        return 1;
    }

    if (!checkCuda(cudaDeviceSynchronize(), "cudaDeviceSynchronize")) {
        cudaFree(deviceS);
        cudaFree(deviceT);
        cudaFree(deviceDifferent);
        return 1;
    }

    if (!checkCuda(cudaMemcpy(hostDifferent, deviceDifferent, intBytes, cudaMemcpyDeviceToHost), "cudaMemcpy device to host")) {
        cudaFree(deviceS);
        cudaFree(deviceT);
        cudaFree(deviceDifferent);
        return 1;
    }

    for (int i = 0; i < count; i++) {
        distance += hostDifferent[i];
    }

    printf("%d\n", distance);

    cudaFree(deviceS);
    cudaFree(deviceT);
    cudaFree(deviceDifferent);

    return 0;
}
