#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <kernel/vmm.h>

extern uint32_t end;
uint32_t placement_address = (uint32_t)&end;
heap_t *kernel_heap;
bool heap_created = false;

void heap_initialize(uint32_t beginning, uint32_t size) {
    if (size <= sizeof(heap_header_t)) {
        kerror("Cannot create heap with smaller than one bite");
        abort();
    }

    // Heap needs to be page aligned
    kernel_heap = (heap_t*)beginning;
    heap_header_t *first_header = (heap_header_t*)(beginning + sizeof(heap_t));

    memset(first_header, 0, sizeof(heap_header_t));
    memset(kernel_heap, 0, sizeof(heap_t));

    first_header->magic = HEAP_HEADER_MAGIC;
    first_header->allocated = 0;
    first_header->size = size - sizeof(heap_header_t);

    kernel_heap->beginning = beginning;
    kernel_heap->size = size;
    kernel_heap->first = first_header;

    printf("Heap information: \n");
    printf("\t-Heap: 0x%X\n", kernel_heap);
    printf("\t-Size: 0x%X\n", kernel_heap->size);
    printf("\t-First Header: 0x%X\n", kernel_heap->first);
    printf("\t-First Magic: 0x%X\n", kernel_heap->first->magic);
    printf("\t-First Size: 0x%X (expecting 0x%X)\n", kernel_heap->first->size,
            (kernel_heap->size - sizeof(heap_header_t)));

    heap_created = true;
}

// Increment the current allocation by size and return
uint32_t kmalloc(size_t size) {
    if (!heap_created) {
        uint32_t tmp = placement_address;

        placement_address += size;

        return tmp;
    } else {
        printf("KHEAP: 0x%X\n", kernel_heap->size);
    }
}

uint32_t kmalloc_aligned(size_t size, bool align) {
    if (!heap_created) {
        // Page align the address if it is not already
        if (align && (placement_address & 0x00000FFF)) {
            placement_address &= 0xFFFFF000;
            placement_address += 0x1000; 
        }

        uint32_t tmp = placement_address;
        placement_address += size;

        return tmp;
    } else {
        printf("KHEAP: 0x%X\n", kernel_heap->size);
    }
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
        
        return tmp;
    } else {
        printf("KHEAP: 0x%X\n", kernel_heap->size);
    }
}
