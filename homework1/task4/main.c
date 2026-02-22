#include <stdio.h>

int main() {
    int a = 10;
    int *p = &a;
    int **pp = &p;

    printf("Value of a: %d\n", a);
    printf("Value of a using pointer: %d\n", *p);
    printf("Value of a using double pointer: %d\n", **pp);

    return 0;
}