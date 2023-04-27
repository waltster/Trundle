#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <kernel/vmm.h>

extern uint32_t end;

uint32_t placement_address = (uint32_t)&end;

// Increment the current allocation by size and return
uint32_t kmalloc(size_t size) {
    uint32_t tmp = placement_address;

    placement_address += size;

    return tmp;
}

uint32_t kmalloc_aligned(size_t size, bool align) {
    // Page align the address if it is not already
    if (align && (placement_address & 0x00000FFF)) {
        placement_address &= 0xFFFFF000;
        placement_address += 0x1000; 
    }

    uint32_t tmp = placement_address;
    placement_address += size;

    return tmp;
}

uint32_t kmalloc_physical_aligned(size_t size, bool align, uint32_t *physical) {
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
}
