#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

int num_players = 5;

pthread_barrier_t ready_barrier;

void *player_thread(void *arg)
{
    int player_id = *(int *)arg;

    srand(time(NULL) + player_id);
    int ready_time = rand() % 5 + 1;

    printf("  Player %d is getting ready... (%d seconds)\n", player_id, ready_time);
    sleep(ready_time);
    printf("  Player %d is READY!\n", player_id);

    int rc = pthread_barrier_wait(&ready_barrier);

    if (rc == PTHREAD_BARRIER_SERIAL_THREAD) {
        printf("\n  All players are ready. Game Started!\n\n");
    }

    return NULL;
}


int main() {
    pthread_barrier_init(&ready_barrier, NULL, num_players);

    pthread_t *threads = malloc(num_players * sizeof(pthread_t));
    int *player_ids = malloc(num_players * sizeof(int));

    printf("Game Lobby: Waiting for %d players\n\n", num_players);

    for (int i = 0; i < num_players; i++) {
        player_ids[i] = i;
        pthread_create(&threads[i], NULL, player_thread, &player_ids[i]);
    }

    for (int i = 0; i < num_players; i++) {
        pthread_join(threads[i], NULL);
    }

    pthread_barrier_destroy(&ready_barrier);
    free(threads);
    free(player_ids);

    return 0;
}