#ifndef _GDT_H
#define _GDT_H 1

#include <stdint.h>

#define GDT_MAX_ENTRIES 5
#define MEMORY_MAX 0xFFFFFFFF

void gdt_initialize();
void gdt_make_entry(int index, uint32_t base, uint32_t limit, uint8_t access, 
        uint8_t flags);
extern void gdt_flush(uint32_t ptr);

// Actual GDT entry structure
typedef struct {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t access;
    uint8_t granularity;
    uint8_t base_high;
} __attribute__((packed)) gdt_entry_t;

// This is passed to the CPU pointing to the first entry to the GDT
typedef struct {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) gdt_ptr_t;

#endif
