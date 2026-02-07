#include <stdio.h>

int str_length(char *str) {
    int length = 0;
    char *p = str;

    while (*p != '\0') {
        length++;
        p++;
    }

    return length;
}

int main() {
    char *str = "Parallel";
    char *p = str;

    while (*p != '\0') {
        printf("%c", *p);
        p++;
    }

    printf("\n");

    printf("Length: %d",str_length(str));

    return 0;
}