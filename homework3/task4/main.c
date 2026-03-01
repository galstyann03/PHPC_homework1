#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

int num_threads = 4;

pthread_barrier_t barrier1;
pthread_barrier_t barrier2;
pthread_barrier_t barrier3;

void *worker_thread(void *arg)
{
    int id = *(int *)arg;
    srand(time(NULL) + id);

    // Stage 1: Data Loading
    int load_time = rand() % 3 + 1;
    printf("  Thread %d: Loading data... (%d seconds)\n", id, load_time);
    sleep(load_time);
    printf("  Thread %d: Stage 1 complete\n", id);

    pthread_barrier_wait(&barrier1);

    if (id == 0)
        printf("\n  All threads finished Stage 1 (Data Loading)\n\n");

    pthread_barrier_wait(&barrier2);

    // Stage 2: Processing
    int process_time = rand() % 3 + 1;
    printf("  Thread %d: Processing data... (%d seconds)\n", id, process_time);
    sleep(process_time);
    printf("  Thread %d: Stage 2 complete\n", id);

    pthread_barrier_wait(&barrier1);

    if (id == 0)
        printf("\n  All threads finished Stage 2 (Processing)\n\n");

    pthread_barrier_wait(&barrier2);

    // Stage 3: Saving Results
    int save_time = rand() % 2 + 1;
    printf("  Thread %d: Saving results... (%d seconds)\n", id, save_time);
    sleep(save_time);
    printf("  Thread %d: Stage 3 complete\n", id);

    pthread_barrier_wait(&barrier3);

    if (id == 0)
        printf("\n  All threads finished Stage 3 (Saving Results)\n\n");

    return NULL;
}

int main() {
    pthread_barrier_init(&barrier1, NULL, num_threads);
    pthread_barrier_init(&barrier2, NULL, num_threads);
    pthread_barrier_init(&barrier3, NULL, num_threads);

    pthread_t *threads = malloc(num_threads * sizeof(pthread_t));
    int *ids = malloc(num_threads * sizeof(int));

    printf("Pipeline: %d threads, 3 stages\n\n", num_threads);

    for (int i = 0; i < num_threads; i++) {
        ids[i] = i;
        pthread_create(&threads[i], NULL, worker_thread, &ids[i]);
    }

    for (int i = 0; i < num_threads; i++)
        pthread_join(threads[i], NULL);

    printf("Pipeline complete!\n");

    pthread_barrier_destroy(&barrier1);
    pthread_barrier_destroy(&barrier2);
    pthread_barrier_destroy(&barrier3);
    free(threads);
    free(ids);

    return 0;
}