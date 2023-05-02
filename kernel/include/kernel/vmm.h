#ifndef _VMM_H
#define _VMM_H 1

#include <stddef.h>
#include <stdbool.h>

#define MEM_MAGIC 0xFEEFD00B
#define HEAP_START         0xC0000000
#define HEAP_INITIAL_SIZE  0x100000
#define MEM_MIN_EXPONENT 8
#define MEM_MAX_EXPONENT 32
#define MEM_MAX_COMPLETE 5

typedef struct mem_tag {
    uint32_t magic;
    uint32_t size;
    uint32_t real_size;
    int index;
    struct mem_tag *split_left;
    struct mem_tag *split_right;
    struct mem_tag *next;
    struct mem_tag *prev;
} mem_tag_t;

void *kmalloc(size_t size);
void *krealloc(void *, size_t);
void *kcalloc(size_t, size_t);
void kfree(void *);

uint32_t _kmalloc(size_t size);
uint32_t _kmalloc_aligned(size_t size, bool align);
uint32_t _kmalloc_physical_aligned(size_t size, bool align, uint32_t *physical);

#endif
