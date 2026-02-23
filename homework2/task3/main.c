#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

#define SIZE 50000000
#define NUM_THREADS 4

int *arr;

struct ThreadData {
    long start;
    long end;
    int local_max;
};

void *find_max(void *arg) {
    struct ThreadData *data = arg;
    int max = arr[data->start];

    for (long i = data->start + 1; i < data->end; i++) {
        if (arr[i] > max) {
            max = arr[i];
        }
    }

    data->local_max = max;
    return NULL;
}

int main() {
    arr = malloc(SIZE * sizeof(int));
    srand(time(NULL));

    for (long i = 0; i < SIZE; i++) {
        arr[i] = rand() * 32768 + rand();
    }

    // Sequential Max
    clock_t start = clock();
    int seq_max = arr[0];
    for (long i = 1; i < SIZE; i++) {
        if (arr[i] > seq_max) {
            seq_max = arr[i];
        }
    }
    clock_t end = clock();

    printf("Sequential max: %d\n", seq_max);
    printf("Sequential time: %.4f seconds\n", (double)(end - start) / CLOCKS_PER_SEC);

    // Parallel Max
    start = clock();

    long chunk = SIZE / NUM_THREADS;
    pthread_t threads[NUM_THREADS];
    struct ThreadData data[NUM_THREADS];

    for (long i = 0; i < NUM_THREADS; i++) {
        data[i].start = i * chunk;
        if (i == NUM_THREADS - 1) {
            data[i].end = SIZE;
        } else {
            data[i].end  = (i + 1) * chunk;
        }
        pthread_create(&threads[i], NULL, find_max, &data[i]);
    }

    int global_max = 0;
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
        if (i == 0 || data[i].local_max > global_max) {
            global_max = data[i].local_max;
        }
    }

    end = clock();

    printf("Parallel max: %d\n", global_max);
    printf("Parallel time: %.4f seconds\n", (double)(end - start) / CLOCKS_PER_SEC);

    free(arr);
    return 0;
}
