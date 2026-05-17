#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>
#include <errno.h>
#include <math.h>

#define SIZE 1000

int *A;

int main() {
    A = malloc(SIZE * sizeof(int));
    if (A == NULL) {
        perror("Failed to allocate memory for A");
        return 1;
    }

    srand(time(NULL));
    for (int i = 0; i < SIZE; i++) {
        A[i] = rand() % 1000;
    }

    int max = A[0];
    #pragma omp parallel for reduction(max:max)
    for (int i = 1; i < SIZE; i++) {
        if (A[i] > max) {
            max = A[i];
        }
    }

    printf("Maximum value: %d\n", max);

    double threshold = max * 0.8;

    int sum = 0;
    #pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < SIZE; i++) {
        if (A[i] > threshold) {
            sum += A[i];
        }
    }

    printf("Sum of values greater than 80%% of max: %d\n", sum);

    free(A);

    return 0;
}