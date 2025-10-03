#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "space_replace.h"


void space_replace(char *s) {
    for (size_t i = 0; i < strlen(s); i++) {
        if (s[i] == ' ') {
            s[i] = '_';
        }
    }
}