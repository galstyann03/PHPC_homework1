#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

#define SIZE 50000000
#define NUM_THREADS 4

int *arr;
long start_index[NUM_THREADS];
long end_index[NUM_THREADS];

void *sum_chunk(void *arg) {
    long id = *(long *)arg;
    long long sum = 0;

    for (long i = start_index[id]; i < end_index[id]; i++) {
        sum += arr[i];
    }

    long long *result = malloc(sizeof(long long));
    *result = sum;
    return result;
}

int main() {
    arr = malloc(SIZE * sizeof(int));
    srand(time(NULL));

    for (long i = 0; i < SIZE; i++) {
        arr[i] = rand() % 100;
    }

    // Sequential Sum
    clock_t start = clock();
    long long total = 0;
    for (long i = 0; i < SIZE; i++) {
        total += arr[i];
    }
    clock_t end = clock();

    printf("Sequential sum: %lld\n", total);
    printf("Sequential time: %.4f seconds\n", (double)(end - start) / CLOCKS_PER_SEC);

    // Parallel Sum
    start = clock();

    long chunk = SIZE / NUM_THREADS;
    pthread_t threads[NUM_THREADS];
    long ids[NUM_THREADS];

    for (long i = 0; i < NUM_THREADS; i++) {
        start_index[i] = i * chunk;
        if (i == NUM_THREADS - 1) {
            end_index[i] = SIZE;
        } else {
            end_index[i] = (i + 1) * chunk;
        }
        ids[i] = i;
        pthread_create(&threads[i], NULL, sum_chunk, &ids[i]);
    }

    // Collect results
    long long parallel_total = 0;
    for (int i = 0; i < NUM_THREADS; i++) {
        void *ret;
        pthread_join(threads[i], &ret);
        parallel_total += *(long long *)ret;
        free(ret);
    }

    end = clock();

    printf("Parallel sum: %lld\n", parallel_total);
    printf("Parallel time: %.4f seconds\n", (double)(end - start) / CLOCKS_PER_SEC);

    free(arr);
    return 0;
}
