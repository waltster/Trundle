#include <stdio.h>
#include <stdint.h>
#include <arch/i386/gdt.h>

gdt_ptr_t gdt_ptr;
gdt_entry_t gdt_entries[GDT_MAX_ENTRIES];

void gdt_make_entry(int index, uint32_t base, uint32_t limit, uint8_t access,
       uint8_t flags) {
    if (index >= GDT_MAX_ENTRIES) {
       kerror("Cannot create GDT entry higher than maximum."); 
       abort();
    }

    gdt_entries[index].limit_low = (limit & 0xFFFF);
    gdt_entries[index].base_low = (base & 0xFFFF);
    gdt_entries[index].base_middle = (base >> 16) & 0xFF;
    gdt_entries[index].base_high = (base >> 24) & 0xFF;;
    gdt_entries[index].access = access;

    // The granularity is only the top 4 bits
    gdt_entries[index].granularity = flags & 0xF0;
}

void gdt_initialize() {
    gdt_ptr.limit = (sizeof(gdt_entry_t) * 5) - 1;
    gdt_ptr.base = (uint32_t)&gdt_entries;

    gdt_make_entry(0, 0, 0, 0, 0);                  // Null
    gdt_make_entry(1, 0, MEMORY_MAX, 0x9A, 0xCF);   // Kernel code
    gdt_make_entry(2, 0, MEMORY_MAX, 0x92, 0xCF);   // Kernel data
    gdt_make_entry(3, 0, MEMORY_MAX, 0xFA, 0xCF);   // User code
    gdt_make_entry(4, 0, MEMORY_MAX, 0xF2, 0xCF);   // User data

    gdt_flush((uint32_t)&gdt_ptr);
}
