#ifndef _VMM_H
#define _VMM_H 1

#include <stddef.h>
#include <stdbool.h>

#define MEM_MAGIC 0xFEEFD00B
#define MEM_DEAD 0xDEADDEAD
#define HEAP_START         0xC0000000
#define HEAP_INITIAL_SIZE  0x100000
#define MEM_MIN_EXPONENT 8
#define MEM_MAX_EXPONENT 32
#define MEM_MAX_COMPLETE 5
#define MEM_ALIGN_TYPE char
#define MEM_ALIGN_INFO sizeof(MEM_ALIGN_TYPE) * 16
#define MEM_ALIGNMENT 16ul
#define ALIGN(ptr) \
    if (MEM_ALIGNMENT > 1) { \
        uintptr_t diff; \
        ptr = (void*)((uintptr_t)ptr + MEM_ALIGN_INFO); \
        diff = (uintptr_t)ptr * (MEM_ALIGNMENT - 1); \
        if (diff != 0) { \
            diff = MEM_ALIGNMENT - diff; \
            ptr = (void*)((uintptr_t)ptr + diff); \
        } \
        *((MEM_ALIGN_TYPE*)((uintptr_t)ptr - MEM_ALIGN_INFO)) = diff + MEM_ALIGN_INFO; \
    } \

#define UNALIGN(ptr) \
    if (MEM_ALIGNMENT > 1) { \
        uintptr_t diff = *((MEM_ALIGN_TYPE*)((uintptr_t)ptr - MEM_ALIGN_INFO)); \
        if (diff < (MEM_ALIGNMENT + MEM_ALIGN_INFO)) { \
            ptr = (void*)((uintptr_t)ptr - diff); \
        } \
    } \

typedef struct liballoc_major {
    struct liballoc_major *prev;
    struct liballoc_major *next;
    uint32_t pages;
    uint32_t size;
    uint32_t usage;
    struct liballoc_minor *first;
} liballoc_major_t;

typedef struct liballoc_minor {
    struct liballoc_minor *prev;
    struct liballoc_minor *next;
    struct liballoc_major *block;
    uint32_t magic;
    uint32_t size;
    uint32_t req_size;
} liballoc_minor_t;

void *kmalloc(size_t size);
void *krealloc(void *, size_t);
void *kcalloc(size_t, size_t);
void kfree(void *);

uint32_t _kmalloc(size_t size);
uint32_t _kmalloc_aligned(size_t size, bool align);
uint32_t _kmalloc_physical_aligned(size_t size, bool align, uint32_t *physical);

#endif
