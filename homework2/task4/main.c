#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

#define LIMIT 20000000
#define NUM_THREADS 4

struct ThreadData {
    long start;
    long end;
    long count;
};

int is_prime(long n) {
    if (n < 2) return 0;
    if (n == 2) return 1;
    if (n % 2 == 0) return 0;
    for (long i = 3; i * i <= n; i += 2) {
        if (n % i == 0) return 0;
    }
    return 1;
}

void *count_primes(void *arg) {
    struct ThreadData *data = arg;
    long count = 0;

    for (long i = data->start; i <= data->end; i++) {
        if (is_prime(i)) {
            count++;
        }
    }

    data->count = count;
    return NULL;
}

int main() {
    // Sequential
    clock_t start = clock();
    long seq_count = 0;
    for (long i = 1; i <= LIMIT; i++) {
        if (is_prime(i)) {
            seq_count++;
        }
    }
    clock_t end = clock();

    printf("Sequential count: %ld\n", seq_count);
    printf("Sequential time: %.4f seconds\n", (double)(end - start) / CLOCKS_PER_SEC);

    // Parallel
    start = clock();

    long chunk = LIMIT / NUM_THREADS;
    pthread_t threads[NUM_THREADS];
    struct ThreadData data[NUM_THREADS];

    for (long i = 0; i < NUM_THREADS; i++) {
        data[i].start = i * chunk + 1;
        if (i == NUM_THREADS - 1) {
            data[i].end = LIMIT;
        } else {
            data[i].end  = (i + 1) * chunk;
        }
        pthread_create(&threads[i], NULL, count_primes, &data[i]);
    }

    long total_count = 0;
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
        total_count += data[i].count;
    }

    end = clock();

    printf("Parallel count: %ld\n", total_count);
    printf("Parallel time: %.4f seconds\n", (double)(end - start) / CLOCKS_PER_SEC);

    return 0;
}
