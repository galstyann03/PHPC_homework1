#include <omp.h>
#include <stdio.h>

#define CUTOFF 10

long long fibonacci_sequential(int num)
{
    if (num <= 1) {
        return num;
    }

    return fibonacci_sequential(num - 1) + fibonacci_sequential(num - 2);
}

long long fibonacci_task(int num)
{
    if (num <= CUTOFF) {
        return fibonacci_sequential(num);
    }

    long long first = 0;
    long long second = 0;

#pragma omp task shared(first) firstprivate(num)
    first = fibonacci_task(num - 1);

#pragma omp task shared(second) firstprivate(num)
    second = fibonacci_task(num - 2);

#pragma omp taskwait
    return first + second;
}

int main(void)
{
    int num;
    int threads = 0;
    long long result = 0;
    double start;
    double end;

    printf("OpenMP Task Fibonacci Program\n");
    printf("Enter num: ");
    if (scanf("%d", &num) != 1) {
        printf("Invalid input\n");
        return 1;
    }

    if (num < 0) {
        printf("num must be non-negative\n");
        return 1;
    }

    if (num > 92) {
        printf("num is too large for long long\n");
        return 1;
    }

    start = omp_get_wtime();

#pragma omp parallel
    {
#pragma omp single
        {
            threads = omp_get_num_threads();
            result = fibonacci_task(num);
        }
    }

    end = omp_get_wtime();

    printf("\nResult\n");
    printf("num = %d\n", num);
    printf("cutoff = %d\n", CUTOFF);
    printf("threads = %d\n", threads);
    printf("F(%d) = %lld\n", num, result);
    printf("time = %.6f seconds\n", end - start);

    return 0;
}
