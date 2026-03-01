#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

int num_players = 3;
int num_rounds = 5;
int *rolls;
int *wins;

pthread_barrier_t roll_barrier;
pthread_barrier_t result_barrier;
pthread_mutex_t rand_mutex;

void *player_thread(void *arg)
{
    int player_id = *(int *)arg;
    srand(time(NULL) + player_id);
    int round;

    for (round = 0; round < num_rounds; round++) {
        int dice = rand() % 6 + 1;

        rolls[player_id] = dice;

        printf("  Round %d: Player %d rolled %d\n", round + 1, player_id, dice);

        int rc = pthread_barrier_wait(&roll_barrier);

        if (rc == PTHREAD_BARRIER_SERIAL_THREAD) {
            int max_roll = 0;
            int i;

            for (i = 0; i < num_players; i++) {
                if (rolls[i] > max_roll)
                    max_roll = rolls[i];
            }

            int tie_count = 0;
            int winner = -1;
            for (i = 0; i < num_players; i++) {
                if (rolls[i] == max_roll) {
                    tie_count++;
                    winner = i;
                }
            }

            if (tie_count > 1) {
                printf(" Round %d: Tie! (rolled %d)\n\n", round + 1, max_roll);
            } else {
                wins[winner]++;
                printf(" Round %d winner: Player %d (rolled %d)\n\n", round + 1, winner, max_roll);
            }
        }

        pthread_barrier_wait(&result_barrier);
    }

    return NULL;
}


int main() {
    rolls = (int *)malloc(num_players * sizeof(int));
    wins = (int *)malloc(num_players * sizeof(int));
    for (int i = 0; i < num_players; i++)
        wins[i] = 0;

    pthread_mutex_init(&rand_mutex, NULL);
    pthread_barrier_init(&roll_barrier,   NULL, num_players);
    pthread_barrier_init(&result_barrier, NULL, num_players);

    pthread_t *threads = malloc(num_players * sizeof(pthread_t));

    int *player_ids = malloc(num_players * sizeof(int));
    for (int i = 0; i < num_players; i++)
        player_ids[i] = i;

    printf("Dice Game: %d players, %d rounds\n\n", num_players, num_rounds);

    for (int i = 0; i < num_players; i++) {
        pthread_create(&threads[i], NULL, player_thread, &player_ids[i]);
    }

    for (int i = 0; i < num_players; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("Final Results\n");
    int max_wins = 0;
    int overall_winner = 0;

    for (int i = 0; i < num_players; i++) {
        printf("  Player %d: %d wins\n", i, wins[i]);
        if (wins[i] > max_wins) {
            max_wins = wins[i];
            overall_winner = i;
        }
    }
    printf("\nOverall Winner: Player %d with %d wins!\n", overall_winner, max_wins);

    pthread_barrier_destroy(&roll_barrier);
    pthread_barrier_destroy(&result_barrier);
    pthread_mutex_destroy(&rand_mutex);
    free(rolls);
    free(wins);
    free(threads);
    free(player_ids);

    return 0;
}