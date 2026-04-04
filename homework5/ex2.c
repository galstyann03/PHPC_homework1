#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <omp.h>

#define NUM_ORDERS  10000
#define NUM_THREADS 4

typedef enum { HIGH, NORMAL } Priority;

typedef struct {
    int order_id;
    float distance_km;
} Order;

int main() {
    Order *orders = malloc(NUM_ORDERS * sizeof(Order));
    Priority *priorities = malloc(NUM_ORDERS * sizeof(Priority));

    if (!orders) {
        perror("malloc orders");
        return -1;
    }
    if (!priorities) {
        perror("malloc priorities");
        free(orders);
        return -1;
    }

    int thread_high_count[NUM_THREADS] = {0};
    int threshold = 0;

    #pragma omp parallel num_threads(NUM_THREADS)
    {
        int tid = omp_get_thread_num();

        #pragma omp single
        {
            threshold = 20;
            for (int i = 0; i < NUM_ORDERS; i++) {
                orders[i].order_id = i + 1;
                orders[i].distance_km  = (float)(rand() % 100);
            }
        }

        #pragma omp for schedule(static)
        for (int i = 0; i < NUM_ORDERS; i++) {
            priorities[i] = (orders[i].distance_km < threshold) ? HIGH : NORMAL;
        }

        #pragma omp barrier

        #pragma omp single
        {
            printf("Priority assignment finished.\n");
        }

        #pragma omp for schedule(static)
        for (int i = 0; i < NUM_ORDERS; i++) {
            if (priorities[i] == HIGH)
                thread_high_count[tid]++;
        }

        #pragma omp barrier

        #pragma omp single
        {
            int total = 0;
            for (int t = 0; t < NUM_THREADS; t++) {
                printf("Thread %d HIGH count: %d\n", t, thread_high_count[t]);
                total += thread_high_count[t];
            }
            printf("Total HIGH priority orders: %d\n", total);
            printf("Total NORMAL priority orders: %d\n", NUM_ORDERS - total);
        }
    }

    free(orders);
    free(priorities);
    return 0;
}