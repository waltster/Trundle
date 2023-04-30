#include <string.h>

void *memcpy(void* restrict dst, const void* restrict src, size_t size) {
    const unsigned char *src_ptr = (const unsigned char*)src;
    unsigned char *dst_ptr = (unsigned char*)dst;

    for(; size != 0; size--) {
        *dst_ptr++ = *src_ptr++;
    }
}
