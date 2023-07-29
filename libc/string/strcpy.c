#include <string.h>

char *strncpy(char *dst, const char *src, size_t len) {
    char *buf = dst;

    while (len && *src != '\0') {
        *buf = *src;
        buf++;
        src++;
        len--;
    }

    while (len) {
        *buf = '\0';
        len--;
    }

    return dst;
}

// TODO: Implement
size_t strlcpy(char * restrict dst, const char * restrict src, size_t dstsize)
{
    return -1;    
}

// TODO: Implement
size_t strlcat(char * restrict dst, const char * restrict src, size_t dstsize)
{
    return -1;
}
