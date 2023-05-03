#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

void *kmalloc(size_t size) {
    return NULL;
}

void kfree(void *ptr) { 
    return;
}

void *kcalloc(size_t count, size_t size) {
    return NULL;
}

void *krealloc(void *ptr) {
    return NULL;
}

void *valloc(size_t size) {
    return NULL;
}

void *aligned_alloc(size_t alignment, size_t size) {
    return NULL;
}
