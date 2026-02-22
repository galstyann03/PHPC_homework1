#include <stdio.h>
#include <stdlib.h>
#define SIZE 5

int main() {
    int *p = malloc(sizeof(int));
    *p = 10;

    printf("%d\n", *p);

    int *arr = malloc(SIZE * sizeof(int));
    for (int i = 0; i < 5; i++) {
        *(arr + i) = i;
    }

    for (int i = 0; i < 5; i++) {
        printf("%d ", *(arr + i));
    }

    free(p);
    free(arr);

    return 0;
}
