#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>
#include <errno.h>
#include <math.h>

#define SIZE 50000000

double *A;

int main( ) {
    A = malloc(SIZE * sizeof(double));
    if (A == NULL) {
        perror("Failed to allocate memory for A");
        return 1;
    }

    srand(time(NULL));
    for (int i = 0; i < SIZE; i++) {
        A[i] = (double)rand() / RAND_MAX;
    }

    double min_diff = fabs(A[0] - A[1]);
    
    #pragma omp parallel for reduction(min:min_diff)
    for (int i = 0; i < SIZE - 1; i++) {
        double diff = fabs(A[i] - A[i + 1]);
        if (diff < min_diff) {
            min_diff = diff;
        }
    }

    printf("Minimum difference: %f\n", min_diff);

    free(A);

    return 0;
}