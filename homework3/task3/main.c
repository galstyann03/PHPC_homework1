#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

int num_sensors = 4;
int num_periods = 3;
float *temperatures;

pthread_barrier_t collect_barrier;
pthread_barrier_t process_barrier;

void *sensor_thread(void *arg)
{
    int sensor_id = *(int *)arg;
    srand(time(NULL) + sensor_id);

    for (int period = 0; period < num_periods; period++) {
        float temp = (rand() % 351 - 50) / 10.0;

        temperatures[sensor_id] = temp;
        printf("  Period %d: Sensor %d collected %.1f C\n", period + 1, sensor_id, temp);

        int rc = pthread_barrier_wait(&collect_barrier);

        if (rc == PTHREAD_BARRIER_SERIAL_THREAD) {
            float sum = 0;
            for (int i = 0; i < num_sensors; i++)
                sum += temperatures[i];

            float avg = sum / num_sensors;
            printf("  >> Period %d average temperature: %.1f C\n\n", period + 1, avg);
        }

        pthread_barrier_wait(&process_barrier);
    }

    return NULL;
}


int main()
{
    temperatures = (float *)malloc(num_sensors * sizeof(float));

    pthread_barrier_init(&collect_barrier, NULL, num_sensors);
    pthread_barrier_init(&process_barrier, NULL, num_sensors);

    pthread_t *threads = malloc(num_sensors * sizeof(pthread_t));
    int *sensor_ids = malloc(num_sensors * sizeof(int));

    printf("Weather Station: %d sensors, %d periods\n\n", num_sensors, num_periods);

    for (int i = 0; i < num_sensors; i++) {
        sensor_ids[i] = i;
        pthread_create(&threads[i], NULL, sensor_thread, &sensor_ids[i]);
    }

    for (int i = 0; i < num_sensors; i++) {
        pthread_join(threads[i], NULL);
    }

    pthread_barrier_destroy(&collect_barrier);
    pthread_barrier_destroy(&process_barrier);
    free(temperatures);
    free(threads);
    free(sensor_ids);

    return 0;
}