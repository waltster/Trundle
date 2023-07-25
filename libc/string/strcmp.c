#include <string.h>
 
int strcmp(const char *str1, const char *str2) {
    while (*str1 != '\0' && *str1 == *str2) {
        str1++; 
        str2++;
    }

    return (int)((uint8_t)*str1 - (uint8_t)*str2);
}

int strncmp(const char *str1, const char *str2, size_t maxlen) {
    while (*str1 != '\0' && *str1 == *str2 && maxlen > 0) {
        str1++;
        str2++;
        maxlen--;
    }

    return (maxlen == 0 ? 0 : (int)((uint8_t)*str1 - (uint8_t)*str2));
}
