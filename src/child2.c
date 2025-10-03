#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "space_replace.h"


int main() {
    char *str1 = NULL;
    size_t len = 0;
    if (getline(&str1, &len, stdin) == -1) {
        perror("Ошибка чтения строки");
        return 1;
    }
    space_replace(str1);
    printf("%s", str1);
    free(str1);
    return 0;
}