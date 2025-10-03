#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "string_to_lowercase.h"


void string_to_lowercase(char *s) {
    for (size_t i = 0; i < strlen(s); i++) {
        if (s[i] >= 'A' && s[i] <= 'Z') {
            s[i] = tolower(s[i]);
        }
    }
}