#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <sched.h>

#define NUM_THREADS 4
#define ITERATIONS 500000000

struct ThreadData {
    int id;
};

void *heavy_work(void *arg) {
    struct ThreadData *data = arg;

    printf("Thread %d started on CPU %d\n", data->id, sched_getcpu());

    long sum = 0;
    for (long i = 0; i < ITERATIONS; i++) {
        sum += i;
    }

    printf("Thread %d finished on CPU %d\n", data->id, sched_getcpu());

    return NULL;
}

int main() {
    pthread_t threads[NUM_THREADS];
    struct ThreadData data[NUM_THREADS];

    for (int i = 0; i < NUM_THREADS; i++) {
        data[i].id = i;
        pthread_create(&threads[i], NULL, heavy_work, &data[i]);
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}
