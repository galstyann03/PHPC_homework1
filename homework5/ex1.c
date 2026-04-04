#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <omp.h>

#define NUM_LOGS 20000
#define NUM_THREADS 4

typedef enum { FAST, MEDIUM, SLOW } LogClass;

typedef struct {
    int request_id;
    int user_id;
    int response_time_ms;
} LogEntry;

int main() {
    LogEntry *logs = malloc(NUM_LOGS * sizeof(LogEntry));
    if (!logs) {
        perror("malloc logs");
        return -1;
    }

    LogClass *classifications = malloc(NUM_LOGS * sizeof(LogClass));
    if (!classifications) {
        perror("malloc classifications");
        free(logs);
        return -1;
    }

    int fast_count = 0, medium_count = 0, slow_count = 0;

    #pragma omp parallel num_threads(NUM_THREADS)
    {
        #pragma omp single
        {
            for (int i = 0; i < NUM_LOGS; i++) {
                logs[i].request_id = i + 1;
                logs[i].user_id = (i % 1000) + 1;
                logs[i].response_time_ms = rand() % 600;
            }
        }

        #pragma omp barrier

        #pragma omp for schedule(static)
        for (int i = 0; i < NUM_LOGS; i++) {
            int t = logs[i].response_time_ms;
            if (t < 100)  classifications[i] = FAST;
            else if (t <= 300) classifications[i] = MEDIUM;
            else classifications[i] = SLOW;
        }

        #pragma omp barrier

        #pragma omp single
        {
            for (int i = 0; i < NUM_LOGS; i++) {
                switch (classifications[i]) {
                    case FAST:   
                        fast_count++;
                        break;
                    case MEDIUM: 
                        medium_count++;
                        break;
                    case SLOW:
                        slow_count++;
                        break;
                }
            }
            printf("FAST count : %d\n", fast_count);
            printf("MEDIUM count : %d\n", medium_count);
            printf("SLOW count : %d\n", slow_count);
            printf("Total count : %d\n", fast_count + medium_count + slow_count);
        }
    }

    free(logs);
    free(classifications);
    return 0;
}