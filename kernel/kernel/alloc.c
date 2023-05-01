#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <alloc.h>

#define PAGE_SIZE 4096
#define MEM_MIN_EXPONENT 8
#define MEM_MAX_EXPONENT 32

uint32_t mem_allocated = 0;
uint32_t mem_inuse = 0;

static bool mem_initialized = false;
static int mem_page_count = 16;

mem_tag_t *mem_free_pages;

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
    if (mem_free_pages[real_index] != NULL) {
        mem_free_pages[real_index]->prev = tag;
        tag->next = mem_free_pages[real_index];
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

    if ((usage % PAGE_SIZE) != 0) {
        pages += 1;
    }

    if (pages < mem_page_count) {
        pages = mem_page_count;
    }

    mem_tag_t *tag = (mem_tag_t*)pmm_allocate
}
