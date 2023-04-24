#include <string.h>

int memcmp(const void *ptr_1, const void* ptr_2, size_t size) {
    const unsigned char *a = (const unsigned char*)ptr_1;
    const unsigned char *b = (const unsigned char*)ptr_2;

    for (size_t i = 0; i < size; i++) {
        if (a[i] < b[i]) {
            return -1;
        } else if (b[i] < a[i]) {
            return 1;
        }
    }

    return 0;
}
