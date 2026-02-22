#include <stdio.h>

int main() {
    int a = 10;
    int *p = &a;

    printf("Variable: %d\n", a);
    printf("Variable address: %p\n", &a);
    printf("Pointer: %p\n\n", p);

    *p = 20;

    printf("Variable: %d\n", a);
    printf("Variable address: %p\n", &a);
    printf("Pointer: %p\n", p);

    return 0;
}