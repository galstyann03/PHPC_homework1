#include <stdio.h>
#define SIZE 5

int main() {
    int arr[SIZE] = {1, 2, 3, 4, 5};
    int *p = arr;

    for (int i = 0; i < SIZE; i++) {
        printf("%d ", *(p + i));
    }

    printf("\n");

    for (int i = 0; i < SIZE; i++) {
        *(p + i) = *(p + i) * 2;
    }

    for (int i = 0; i < SIZE; i++) {
        printf("%d ", arr[i]);
    }

    return 0;
}
