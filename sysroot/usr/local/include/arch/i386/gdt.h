#ifndef _GDT_H
#define _GDT_H 1

#include <stdint.h>

#define GDT_MAX_ENTRIES 5
#define MEMORY_MAX 0xFFFFFFFF

#define FLAG_ACCESS_ABSENT      0 << 7
#define FLAG_ACCESS_PRESENT     1 << 7
#define FLAG_ACCESS_DPL_0       (0 & 3) << 5
#define FLAG_ACCESS_DPL_1       (1 & 3) << 5
#define FLAG_ACCESS_DPL_2       (2 & 3) << 5
#define FLAG_ACCESS_DPL_3       (3 & 3) << 5
#define FLAG_ACCESS_TYPE_SYSTEM 0 << 4
#define FLAG_ACCESS_TYPE_OTHER  1 << 4
#define FLAG_GRAN_BYTE          0 << 7
#define FLAG_GRAN_KBYTE         1 << 7
#define FLAG_GRAN_OP_16         0 << 6
#define FLAG_GRAN_OP_32         1 << 6
#define FLAG_GRAN_0             0 << 5
#define FLAG_GRAN_AVAIL         0 << 4

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
