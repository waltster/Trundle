#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <kernel/vmm.h>
#include <arch/i386/pmm.h>

extern uint32_t end;
uint32_t mem_allocated = 0;
uint32_t mem_inuse = 0;
uint32_t heap_location = HEAP_START;
static int mem_page_count = 16;
static bool mem_initialized = false;
uint32_t placement_address = (uint32_t)&end;
mem_tag_t *mem_free_pages[MEM_MAX_EXPONENT];
int mem_complete_pages[MEM_MAX_EXPONENT];
extern page_directory_t *pmm_kernel_directory;

static void mem_lock() {
    asm volatile("cli");
}

static void mem_unlock() {
    asm volatile("sti");
}

static inline int mem_get_exponent(size_t size) {
    if (size < (1 << MEM_MIN_EXPONENT)) {
        return -1;
    }

    int shift = MEM_MIN_EXPONENT;

    while (shift < MEM_MAX_EXPONENT) {
        if ((1 << shift) > size) break;

        shift += 1;
    }

    return shift - 1;
}

// 
static void mem_insert_tag(int index, mem_tag_t *tag) {
    int real_index;

    if (index < 0) {
        real_index = mem_get_exponent(tag->real_size - sizeof(mem_tag_t));

        if (real_index < MEM_MIN_EXPONENT) {
            real_index = MEM_MIN_EXPONENT;
        }
    } else {
        real_index = index;
    }

    tag->index = real_index;

    // If the free pages at that index is not null, then go ahead and insert the
    // tag before the existing one.
    if (mem_free_pages[real_index] != (0)) {
        mem_free_pages[real_index]->prev = tag;
        tag->next = (struct mem_tag*)mem_free_pages[real_index];
    }

    mem_free_pages[real_index] = tag;
}

/**
 * Remove the tag from its linked list as an element and then set its index to
 * -1
 */
static void mem_remove_tag(mem_tag_t *tag) {
    if (mem_free_pages[tag->index] == tag) {
        mem_free_pages[tag->index] = tag->next;
    }

    if (tag->prev != NULL) {
        tag->prev->next = tag->next;
    }

    if (tag->next != NULL) {
        tag->next->prev = tag->prev;
    }

    tag->next = NULL;
    tag->prev = NULL;
    tag->index = -1;
}

/*
 * Causes the tag specified to be merged into the tag on its left, if there is
 * one.
 */
static mem_tag_t* mem_cave_left(mem_tag_t *tag) {
    mem_tag_t *left = tag->split_left;

    left->real_size += tag->real_size;
    left->split_right = tag->split_right;

    if (tag->split_right != NULL) {
        tag->split_right->split_left = left;
    }

    return left;
}

/*
 * Absorb the tag to the right of this tag, if there is one.
 */
static mem_tag_t* mem_absorb_right(mem_tag_t *tag) {
    mem_tag_t *right = tag->split_right;

    mem_remove_tag(right);

    tag->real_size += right->real_size;
    tag->split_right = right->split_right;

    if (tag->split_right != NULL) {
        tag->split_right->split_left = tag;
    }

    return tag;
}

static mem_tag_t* mem_split_tag(mem_tag_t* tag) {
    uint32_t remainder = (tag->real_size - sizeof(mem_tag_t) - tag->size);

    // Create a new tag at the end of the current tag, using the remaining
    // space in the allocation
    mem_tag_t *new_tag = (mem_tag_t*)((uint32_t)tag + sizeof(mem_tag_t) + 
            tag->size);

    new_tag->magic = MEM_MAGIC;
    new_tag->real_size = remainder;
    new_tag->next = NULL;
    new_tag->prev = NULL;
    new_tag->split_left = tag;
    new_tag->split_right = tag->split_right;

    if (new_tag->split_right != NULL) {
        new_tag->split_right->split_left = new_tag;
    }

    tag->split_right = new_tag;
    tag->real_size -= new_tag->real_size;

    mem_insert_tag(-1, new_tag);

    return new_tag;
}

static mem_tag_t* mem_create_tag(size_t size) {
    uint32_t memory_required = (size + sizeof(mem_tag_t));
    uint32_t pages = memory_required / PAGE_SIZE;

    if ((memory_required % PAGE_SIZE) != 0) {
        pages += 1;
    }

    if (pages < mem_page_count) {
        pages = mem_page_count;
    }

    mem_tag_t *tag = (mem_tag_t*)pmm_allocate_and_map_for_heap(pages);
    printf("Page at: 0x%X\n", tag);

    if (tag == NULL) return NULL;

    tag->magic = MEM_MAGIC;
    tag->size = size;
    tag->real_size = pages * PAGE_SIZE;
    tag->index = -1;
    tag->next = NULL;
    tag->prev = NULL;
    tag->split_left = NULL;
    tag->split_right = NULL;

    mem_allocated += pages * PAGE_SIZE;

    return tag;
}

void *kmalloc(size_t size) {
    int index;
    void *ptr;
    mem_tag_t *tag = NULL;

    mem_lock();

    if (mem_initialized == false) {
        for (index = 0; index < MEM_MAX_EXPONENT; index++) {
            mem_free_pages[index] = 0;
            mem_complete_pages[index] = 0;
        }

        mem_initialized = true;
    }

    index = mem_get_exponent(size);

    if (index < MEM_MIN_EXPONENT) {
        index = MEM_MIN_EXPONENT;
    }

    tag = mem_free_pages[index];

    while (tag != NULL) {
        if ((tag->real_size - sizeof(mem_tag_t)) >= (size + sizeof(mem_tag_t))) {
            break;
        }

        tag = tag->next;
    }

    if (tag == NULL) {
        if ((tag = mem_create_tag(size)) == NULL) {
            mem_unlock();
            return NULL;
        }

        index = mem_get_exponent(tag->real_size - sizeof(mem_tag_t));
    } else {
        mem_remove_tag(tag);

        if ((tag->split_left == NULL) && (tag->split_right == NULL)) {
            mem_complete_pages[index] -= 1;
        }
    }

    tag->size = size;

    uint32_t remainder = tag->real_size - size - sizeof(mem_tag_t) * 2;

    if ( ((int)remainder) > 0) {
        int child_index = mem_get_exponent(remainder);

        if (child_index >= 0) {
            mem_tag_t *new_tag = mem_split_tag(tag);
            new_tag = new_tag;
        }
    }

    ptr = (void*)((uint32_t)tag + sizeof(mem_tag_t));

    mem_inuse += size;

    mem_unlock();

    printf("Pointer at 0x%X\n", ptr);
    return ptr;
}

void kfree(void *ptr) {
    int index;
    mem_tag_t *tag;

    if (ptr == NULL) return;

    mem_lock();

    tag = (mem_tag_t*)((uint32_t)ptr - sizeof(mem_tag_t));

    if (tag->magic != MEM_MAGIC) {
        mem_unlock();
        return;
    }

    mem_inuse -= tag->size;

    while ((tag->split_left != NULL) && (tag->split_left->index >= 0)) {
        tag = mem_absorb_right(tag);
    }

    index = mem_get_exponent(tag->real_size - sizeof(mem_tag_t));
    
    if (index < MEM_MIN_EXPONENT) {
        index = MEM_MIN_EXPONENT;
    }

    if ((tag->split_left == NULL) && (tag->split_right == NULL)) {
        if (mem_complete_pages[index] == MEM_MAX_COMPLETE) {
            uint32_t pages = tag->real_size / PAGE_SIZE;

            if ((tag->real_size % PAGE_SIZE) != 0) {
                pages += 1;
            }

            if (pages < mem_page_count) {
                pages = mem_page_count;
            }

            pmm_free_pages(tag, pages);
            mem_allocated -= pages * PAGE_SIZE;

            mem_unlock();
            return;
        }

        mem_complete_pages[index] += 1;
    }

    mem_insert_tag(tag, index);

    mem_unlock();
}

void *kcalloc(size_t nobj, size_t size) {
    int real_size = nobj * size;
    void *p;

    p = kmalloc(real_size);

    memset(p, 0, real_size);

    return p;
}

void *krealloc(void *p, size_t size) {
    void *ptr;
    mem_tag_t *tag;
    int real_size;

    if (size == 0) {
        kfree(p);
        return NULL;
    }

    if (p == NULL) {
        return kmalloc(size);
    }

    mem_lock();
    tag = (mem_tag_t*)((uint32_t)p - sizeof(mem_tag_t));
    real_size = tag->size;
    mem_unlock();

    if (real_size > size) {
        real_size = size;
    }

    ptr = kmalloc(size);
    memcpy(ptr, p, real_size);
    kfree(p);

    return ptr;
}

void _kfree(void *ptr) {}

// Increment the current allocation by size and return
uint32_t _kmalloc(size_t size) {
    return (void*)_kmalloc_physical_aligned(size, false, 0);
}

uint32_t _kmalloc_aligned(size_t size, bool align) {
    return (void*)_kmalloc_physical_aligned(size, align, 0);
}

uint32_t _kmalloc_physical_aligned(size_t size, bool align, uint32_t *physical) {
    if (align && (placement_address & 0x00000FFF)) {
        placement_address &= 0xFFFFF000;
        placement_address += 0x1000;
    }

    if (physical) {
        *physical = placement_address;
    }

    // TODO: Replace this with a virtual allocation
    uint32_t tmp = placement_address;
    placement_address += size;
    
    return (void*)tmp;
}
