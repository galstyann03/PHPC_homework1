#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <pthread.h>
#include <immintrin.h>

#define SIZE (256 * 1024 * 1024)
#define NUM_THREADS 4

char *dna;
long mult_thr_countA = 0, mult_thr_countC = 0, mult_thr_countG = 0, mult_thr_countT = 0;
long simd_mult_thr_countA = 0, simd_mult_thr_countC = 0, simd_mult_thr_countG = 0, simd_mult_thr_countT = 0;

pthread_mutex_t lock;

typedef struct {
    long start;
    long end;
} Thread_Arg;

double get_time(struct timespec start, struct timespec end) {
    return (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
}

void *counter(void *arg) {
    Thread_Arg *tharg = (Thread_Arg *)arg;
    long localA = 0, localC = 0, localG = 0, localT = 0;

    for (long i = tharg->start; i < tharg->end; i++) {
        if (dna[i] == 'A') localA++;
        else if (dna[i] == 'C') localC++;
        else if (dna[i] == 'G') localG++;
        else if (dna[i] == 'T') localT++;
    }

    pthread_mutex_lock(&lock);
    mult_thr_countA += localA;
    mult_thr_countC += localC;
    mult_thr_countG += localG;
    mult_thr_countT += localT;
    pthread_mutex_unlock(&lock);

    return NULL;
}

void *simd_counter(void *arg) {
    Thread_Arg *tharg = (Thread_Arg *)arg;
    long localA = 0, localC = 0, localG = 0, localT = 0;

    __m256i vectorA = _mm256_set1_epi8('A');
    __m256i vectorC = _mm256_set1_epi8('C');
    __m256i vectorG = _mm256_set1_epi8('G');
    __m256i vectorT = _mm256_set1_epi8('T');

    for (long i = tharg->start; i < tharg->end; i += 32) {
        __m256i buf = _mm256_loadu_si256((__m256i*)(dna + i));

        localA += __builtin_popcount(_mm256_movemask_epi8(_mm256_cmpeq_epi8(buf, vectorA)));
        localC += __builtin_popcount(_mm256_movemask_epi8(_mm256_cmpeq_epi8(buf, vectorC)));
        localG += __builtin_popcount(_mm256_movemask_epi8(_mm256_cmpeq_epi8(buf, vectorG)));
        localT += __builtin_popcount(_mm256_movemask_epi8(_mm256_cmpeq_epi8(buf, vectorT)));
    }

    for (long i = (SIZE / 32) * 32; i < SIZE; i++) {
        if (dna[i] == 'A') localA++;
        else if (dna[i] == 'C') localC++;
        else if (dna[i] == 'G') localG++;
        else if (dna[i] == 'T') localT++;
    }

    pthread_mutex_lock(&lock);
    simd_mult_thr_countA += localA;
    simd_mult_thr_countC += localC;
    simd_mult_thr_countG += localG;
    simd_mult_thr_countT += localT;
    pthread_mutex_unlock(&lock);

    return NULL;
}

int main() {
    dna = malloc(SIZE);
    char nucleotides[4] = {'A', 'C', 'G', 'T'};
    struct timespec start, end;
    double scalar_time, mult_thr_time, simd_time, simd_mult_thr_time;

    srand(time(NULL));

    for (long i = 0; i < SIZE; i++) {
        *(dna + i) = nucleotides[rand() % 4];
    }

    // Scalar Version
    clock_gettime(CLOCK_MONOTONIC, &start);

    long scal_countA = 0, scal_countC = 0, scal_countG = 0, scal_countT = 0;
    for (long i = 0; i < SIZE; i++) {
        if (dna[i] == 'A') scal_countA++;
        else if (dna[i] == 'C') scal_countC++;
        else if (dna[i] == 'G') scal_countG++;
        else if (dna[i] == 'T') scal_countT++;
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    scalar_time = get_time(start, end);

    // Multithreading Implementation
    clock_gettime(CLOCK_MONOTONIC, &start);

    pthread_t threads[NUM_THREADS];
    pthread_mutex_init(&lock, NULL);
    Thread_Arg thread_arg[NUM_THREADS];
    long chunk = SIZE / NUM_THREADS;

    for (int i = 0; i < NUM_THREADS; i++) {
        thread_arg[i].start = chunk * i;
        if (i + 1 == NUM_THREADS) thread_arg[i].end = SIZE;
        else thread_arg[i].end = (i + 1) * chunk;
        pthread_create(&threads[i], NULL, counter, &thread_arg[i]);
    }

    for (int i = 0; i < NUM_THREADS; i++)
        pthread_join(threads[i], NULL);

    pthread_mutex_destroy(&lock);

    clock_gettime(CLOCK_MONOTONIC, &end);    
    mult_thr_time = get_time(start, end);

    // SIMD Implementation
    clock_gettime(CLOCK_MONOTONIC, &start);

    long simdA = 0, simdC = 0, simdG = 0, simdT = 0;
    __m256i vectorA = _mm256_set1_epi8('A');
    __m256i vectorC = _mm256_set1_epi8('C');
    __m256i vectorG = _mm256_set1_epi8('G');
    __m256i vectorT = _mm256_set1_epi8('T');

    for (long i = 0; i < SIZE; i += 32) {
        __m256i buf = _mm256_loadu_si256((__m256i*)(dna + i));

        simdA += __builtin_popcount(_mm256_movemask_epi8(_mm256_cmpeq_epi8(buf, vectorA)));
        simdC += __builtin_popcount(_mm256_movemask_epi8(_mm256_cmpeq_epi8(buf, vectorC)));
        simdG += __builtin_popcount(_mm256_movemask_epi8(_mm256_cmpeq_epi8(buf, vectorG)));
        simdT += __builtin_popcount(_mm256_movemask_epi8(_mm256_cmpeq_epi8(buf, vectorT)));
    }

    for (long i = (SIZE / 32) * 32; i < SIZE; i++) {
        if (dna[i] == 'A') simdA++;
        else if (dna[i] == 'C') simdC++;
        else if (dna[i] == 'G') simdG++;
        else if (dna[i] == 'T') simdT++;
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    simd_time = get_time(start, end);

    // SIMD + Multithreading implementation
    clock_gettime(CLOCK_MONOTONIC, &start);

    pthread_t simd_threads[NUM_THREADS];
    pthread_mutex_init(&lock, NULL);
    Thread_Arg simd_thread_arg[NUM_THREADS];

    for (int i = 0; i < NUM_THREADS; i++) {
        simd_thread_arg[i].start = chunk * i;
        if (i + 1 == NUM_THREADS) simd_thread_arg[i].end = SIZE;
        else simd_thread_arg[i].end = (i + 1) * chunk;
        pthread_create(&simd_threads[i], NULL, simd_counter, &simd_thread_arg[i]);
    }

    for (int i = 0; i < NUM_THREADS; i++)
        pthread_join(simd_threads[i], NULL);

    pthread_mutex_destroy(&lock);

    clock_gettime(CLOCK_MONOTONIC, &end);
    simd_mult_thr_time = get_time(start, end);

    // Verification
    int correct = 1;
    if (mult_thr_countA != scal_countA || mult_thr_countC != scal_countC ||
        mult_thr_countG != scal_countG || mult_thr_countT != scal_countT) {
        printf("MISMATCH: Multithreading results differ from Scalar!\n");
        correct = 0;
    }
    if (simdA != scal_countA || simdC != scal_countC ||
        simdG != scal_countG || simdT != scal_countT) {
        printf("MISMATCH: SIMD results differ from Scalar!\n");
        correct = 0;
    }
    if (simd_mult_thr_countA != scal_countA || simd_mult_thr_countC != scal_countC ||
        simd_mult_thr_countG != scal_countG || simd_mult_thr_countT != scal_countT) {
        printf("MISMATCH: SIMD+Multithreading results differ from Scalar!\n");
        correct = 0;
    }

    // Program Output
    printf("DNA size: %d MB\n", SIZE / (1024 * 1024));
    printf("Threads used: %d\n\n", NUM_THREADS);
    printf("Counts (A C G T):\n");
    printf("%ld %ld %ld %ld\n\n", scal_countA, scal_countC, scal_countG, scal_countT);
    if (correct) printf("All implementations match!\n\n");
    printf("Scalar Time:                 %.3f sec\n", scalar_time);
    printf("Multithreading time:         %.3f sec\n", mult_thr_time);
    printf("SIMD time:                   %.3f sec\n", simd_time);
    printf("SIMD + Multithreading time:  %.3f sec\n", simd_mult_thr_time);

    free(dna);

    return 0;
}
