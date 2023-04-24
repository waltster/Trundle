#include <string.h>

void *memset(void *ptr, int value, size_t size) {
    unsigned char *buffer = (unsigned char*)ptr;

    for (size_t i = 0; i < size; i++) {
        buffer[i] = (unsigned char) value;
    }

    return ptr;
}
