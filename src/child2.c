#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "space_replace.h"


int main() {
    char *str1 = NULL;
    size_t len = 0;
    ssize_t nread;
    while ((nread = getline(&str1, &len, stdin)) != -1) {
        space_replace(str1);
        if (fwrite(str1, 1, nread, stdout) != (size_t)nread) {
            fflush(stdout);
            perror("Ошибка записи в stdout");
            free(str1);
            return 1;
        }
    }
    free(str1);
    return 0;
}