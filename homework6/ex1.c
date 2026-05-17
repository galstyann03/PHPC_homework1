#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>
#include <errno.h>

#define SIZE 100000000
#define RANGE 256
int *A;
int hist[RANGE];

int main() {
    A = malloc(SIZE * sizeof(int));
    for (int i = 0; i < RANGE; i++) {
        hist[i] = 0;
    }

    if (A == NULL) {
        perror("Failed to allocate memory for A");
        return 1;
    }

    srand(time(NULL));
    for (int i = 0; i < SIZE; i++) {
        A[i] = rand() % RANGE;
    }

    // Naive parallel version (race condition expected)
    // #pragma omp parallel for
    // for (int i = 0; i < SIZE; i++) {
    //     hist[A[i]]++;
    // }

    // #pragma omp parallel for
    // for (int i = 0; i < SIZE; i++) {

    //     #pragma omp critical
    //     {
    //         hist[A[i]]++;
    //     }
    // }

    // Reduction version
    #pragma omp parallel for reduction(+:hist[:RANGE])
    for (int i = 0; i < SIZE; i++) {
        hist[A[i]]++;
    }

    printf("Naive histogram:\n");
    for (int i = 0; i < RANGE; i++) {
        printf("%d: %d\n", i, hist[i]);
    }

    free(A);

    return 0;
}
