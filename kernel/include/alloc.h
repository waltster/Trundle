#ifndef __ALLOC_H
#define __ALLOC_H 1

#include <stdint.h>

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

void *mem_alloc(size_t);
void *mem_realloc(void *,size_t);
void *mem_calloc(size_t, size_t);
void free(void *);

#endif
