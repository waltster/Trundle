#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <kernel/vmm.h>
#include <arch/i386/pmm.h>

extern uint32_t end;
uint32_t heap_location = HEAP_START;
uint32_t placement_address = (uint32_t)&end;
extern page_directory_t *pmm_kernel_directory;

/******************************************************************************
 * Internal, raw allocations.
 *****************************************************************************/

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
