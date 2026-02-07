#include <stdio.h>

int main() {
    char *str[] = {"core", "process", "thread"};

    for (int i = 0; i < 3; i++) {
        printf("%s\n", *(str + i));
    }

    str[2] = "multithreading";

    printf("\n");

    for (int i = 0; i < 3; i++) {
        printf("%s\n", *(str + i));
    }

    return 0;
}
