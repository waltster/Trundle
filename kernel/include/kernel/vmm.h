#ifndef _VMM_H
#define _VMM_H 1

#include <stddef.h>
#include <stdbool.h>

uint32_t kmalloc(size_t size);
uint32_t kmalloc_aligned(size_t size, bool align);
uint32_t kmalloc_physical_aligned(size_t size, bool align, uint32_t *physical);

#endif
