#include <string.h>
 
size_t strlen(const char* str) {
    const char *s;

    for (s = str; *s; ++s);

    return (s - str);
}

size_t strnlen(const char* str, size_t maxlen) {
    size_t len = 0;

    while (str[len] && maxlen) {
        len++;
        maxlen--;
    }

    return len;
}
