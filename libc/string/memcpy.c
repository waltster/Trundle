#include <string.h>

void *memcpy(void* restrict dst, const void* restrict src, size_t size) {
    unsigned char *dst_c = (unsigned char*)dst;
    const unsigned char *src_c = (const unsigned char*)src;

    for (size_t i = 0; i < size; i++) {
        dst_c[i] = src_c[i];
    }

    return dst;
}
