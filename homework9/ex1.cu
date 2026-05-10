#include <stdio.h>
#include <string.h>
#include <cuda_runtime.h>

#define MAX_SIZE 1001

__device__ double getMass(char amino)
{
    if (amino == 'A') return 71.037113805;
    if (amino == 'C') return 103.009184505;
    if (amino == 'D') return 115.026943065;
    if (amino == 'E') return 129.042593135;
    if (amino == 'F') return 147.068413945;
    if (amino == 'G') return 57.021463735;
    if (amino == 'H') return 137.058911875;
    if (amino == 'I') return 113.084064015;
    if (amino == 'K') return 128.094963050;
    if (amino == 'L') return 113.084064015;
    if (amino == 'M') return 131.040484645;
    if (amino == 'N') return 114.042927470;
    if (amino == 'P') return 97.052763875;
    if (amino == 'Q') return 128.058577540;
    if (amino == 'R') return 156.101111050;
    if (amino == 'S') return 87.032028435;
    if (amino == 'T') return 101.047678505;
    if (amino == 'V') return 99.068413945;
    if (amino == 'W') return 186.079312980;
    if (amino == 'Y') return 163.063328575;

    return 0.0;
}

__global__ void calculateMasses(const char *protein, double *masses, int count)
{
    int tid = blockIdx.x * blockDim.x + threadIdx.x;

    if (tid < count) {
        masses[tid] = getMass(protein[tid]);
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
    char hostProtein[MAX_SIZE];
    double hostMasses[MAX_SIZE];
    char *deviceProtein = NULL;
    double *deviceMasses = NULL;
    int blockSize = 256;
    double totalMass = 0.0;

    scanf("%1000s", hostProtein);

    int count = (int)strlen(hostProtein);
    size_t proteinBytes = count * sizeof(char);
    size_t massBytes = count * sizeof(double);
    int gridSize = (count + blockSize - 1) / blockSize;

    if (!checkCuda(cudaMalloc((void**)&deviceProtein, proteinBytes), "cudaMalloc deviceProtein")) {
        return 1;
    }

    if (!checkCuda(cudaMalloc((void**)&deviceMasses, massBytes), "cudaMalloc deviceMasses")) {
        cudaFree(deviceProtein);
        return 1;
    }

    if (!checkCuda(cudaMemcpy(deviceProtein, hostProtein, proteinBytes, cudaMemcpyHostToDevice), "cudaMemcpy host to device")) {
        cudaFree(deviceProtein);
        cudaFree(deviceMasses);
        return 1;
    }

    calculateMasses<<<gridSize, blockSize>>>(deviceProtein, deviceMasses, count);

    if (!checkCuda(cudaGetLastError(), "Kernel launch")) {
        cudaFree(deviceProtein);
        cudaFree(deviceMasses);
        return 1;
    }

    if (!checkCuda(cudaDeviceSynchronize(), "cudaDeviceSynchronize")) {
        cudaFree(deviceProtein);
        cudaFree(deviceMasses);
        return 1;
    }

    if (!checkCuda(cudaMemcpy(hostMasses, deviceMasses, massBytes, cudaMemcpyDeviceToHost), "cudaMemcpy device to host")) {
        cudaFree(deviceProtein);
        cudaFree(deviceMasses);
        return 1;
    }

    for (int i = 0; i < count; i++) {
        totalMass += hostMasses[i];
    }

    printf("%.3f\n", totalMass);

    cudaFree(deviceProtein);
    cudaFree(deviceMasses);

    return 0;
}
