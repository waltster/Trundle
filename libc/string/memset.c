#include <string.h>

void *memset(void *ptr, int value, size_t size) {
    unsigned char *tmp = (unsigned char*)ptr;

    for(; size != 0; size--) {
        *tmp++ = value;
    }

    return ptr;
}
