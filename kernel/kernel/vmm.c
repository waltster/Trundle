#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <kernel/vmm.h>
#include <arch/i386/pmm.h>

extern uint32_t end;
uint32_t placement_address = (uint32_t)&end;
heap_t *kernel_heap;
bool heap_created = false;
extern page_directory_t *pmm_kernel_directory;

void heap_initialize(uint32_t beginning, uint32_t size) {
    if (heap_created) {
        return;
    }

    kernel_heap = (heap_t*)kmalloc(sizeof(heap_t));

    if ((beginning & 0xFFFFF000) != 0) {
        beginning &= 0xFFFFF000;
        beginning += 0x1000;
    }

    kernel_heap->heap_start = beginning;
    kernel_heap->heap_size = size;

    heap_header_t* header = (heap_header_t*)beginning;
    header->size = (size - sizeof(heap_header_t));
    header->magic = HEAP_HEADER_MAGIC;
    header->allocated = 0;

    heap_created = true;
    printf("Heap initialized\n");
}

/*
 * Allocate from the buddy system
 */
void kfree(void *ptr) {}

// Increment the current allocation by size and return
uint32_t kmalloc(size_t size) {
    return (void*)kmalloc_physical_aligned(size, false, 0);
}

uint32_t kmalloc_aligned(size_t size, bool align) {
    return (void*)kmalloc_physical_aligned(size, align, 0);
}

uint32_t kmalloc_physical_aligned(size_t size, bool align, uint32_t *physical) {
    if (!heap_created) {
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
    } else {
        printf("Allocating from heap\n");
        printf("Size: %d, align: 0x%X, heap 0x%X\n", size, align, kernel_heap);
        void *addr = NULL;

        if (physical != 0) {
            page_t *page = pmm_get_page((uint32_t)addr, 0, pmm_kernel_directory);
            *physical = (page->address * 0x1000) + ((uint32_t)addr & 0xFFF);
        }
        printf("Address: 0x%X\n", addr);

        memset(addr, 0, size);
        return (void*)addr;
    }
}
