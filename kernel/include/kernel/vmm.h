#ifndef _VMM_H
#define _VMM_H 1

#include <stddef.h>
#include <stdbool.h>

#define HEAP_HEADER_MAGIC 0xFEEFD00B
#define HEAP_START         0xC0000000
#define HEAP_INITIAL_SIZE  0x100000

typedef struct {
    uint32_t heap_size;
    uint32_t heap_start;
} heap_t;

typedef struct {
    uint32_t magic;
    uint32_t size;
    uint32_t allocated;
} heap_header_t;

void heap_initialize(uint32_t beginning, uint32_t size);
uint32_t kmalloc(size_t size);
uint32_t kmalloc_aligned(size_t size, bool align);
uint32_t kmalloc_physical_aligned(size_t size, bool align, uint32_t *physical);

#endif
