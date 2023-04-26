#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <arch/i386/descriptor_tables.h>

gdt_ptr_t gdt_ptr;
gdt_entry_t gdt_entries[GDT_MAX_ENTRIES];
idt_ptr_t idt_ptr;
idt_entry_t idt_entries[IDT_MAX_ENTRIES];

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

    gdt_flush((void*)&gdt_ptr);
}

void idt_initialize() {
    idt_ptr.size = (sizeof(idt_entry_t) * 256);
    idt_ptr.offset = (uint32_t)&idt_entries;

    memset(&idt_entries, 0, sizeof(idt_entry_t) * 256);
    
    idt_make_entry(0, &isr_0, 0x08, IDT_TRAP_GATE);
    idt_make_entry(1, &isr_1, 0x08, IDT_TRAP_GATE);
    idt_make_entry(2, &isr_2, 0x08, IDT_TRAP_GATE);
    idt_make_entry(3, &isr_3, 0x08, IDT_TRAP_GATE);
    idt_make_entry(4, &isr_4, 0x08, IDT_TRAP_GATE);
    idt_make_entry(5, &isr_5, 0x08, IDT_TRAP_GATE);
    idt_make_entry(6, &isr_6, 0x08, IDT_TRAP_GATE);
    idt_make_entry(7, &isr_7, 0x08, IDT_TRAP_GATE);
    idt_make_entry(8, &isr_8, 0x08, IDT_TRAP_GATE);
    idt_make_entry(9, &isr_9, 0x08, IDT_TRAP_GATE);
    idt_make_entry(10, &isr_10, 0x08, IDT_TRAP_GATE);
    idt_make_entry(11, &isr_11, 0x08, IDT_TRAP_GATE);
    idt_make_entry(12, &isr_12, 0x08, IDT_TRAP_GATE);
    idt_make_entry(13, &isr_13, 0x08, IDT_TRAP_GATE);
    idt_make_entry(14, &isr_14, 0x08, IDT_TRAP_GATE);
    idt_make_entry(15, &isr_15, 0x08, IDT_TRAP_GATE);
    idt_make_entry(16, &isr_16, 0x08, IDT_TRAP_GATE);
    idt_make_entry(17, &isr_17, 0x08, IDT_TRAP_GATE);
    idt_make_entry(18, &isr_18, 0x08, IDT_TRAP_GATE);
    idt_make_entry(19, &isr_19, 0x08, IDT_TRAP_GATE);
    idt_make_entry(20, &isr_20, 0x08, IDT_TRAP_GATE);
    idt_make_entry(21, &isr_21, 0x08, IDT_TRAP_GATE);
    idt_make_entry(22, &isr_22, 0x08, IDT_TRAP_GATE);
    idt_make_entry(23, &isr_23, 0x08, IDT_TRAP_GATE);
    idt_make_entry(24, &isr_24, 0x08, IDT_TRAP_GATE);
    idt_make_entry(25, &isr_25, 0x08, IDT_TRAP_GATE);
    idt_make_entry(26, &isr_26, 0x08, IDT_TRAP_GATE);
    idt_make_entry(27, &isr_27, 0x08, IDT_TRAP_GATE);
    idt_make_entry(28, &isr_28, 0x08, IDT_TRAP_GATE);
    idt_make_entry(29, &isr_29, 0x08, IDT_TRAP_GATE);
    idt_make_entry(30, &isr_30, 0x08, IDT_TRAP_GATE);
    idt_make_entry(31, &isr_31, 0x08, IDT_TRAP_GATE);

    idt_flush((void*)&idt_ptr);
}

void idt_make_entry(int index, uint32_t base, uint16_t selector, 
    uint8_t flags) {
    if (index >= IDT_MAX_ENTRIES) {
        kerror("Cannot exceed the maximum entry index");
        abort();
    }

    idt_entries[index].offset_low = base & 0xFFFF;
    idt_entries[index].offset_high = (base >> 16) & 0xFFFF;
    idt_entries[index].segment_selector = selector;
    idt_entries[index].flags = flags;
    idt_entries[index].reserved = 0;
}

void isr_handler(registers_t* regs) {
    printf("Error #%X\n", regs->interrupt_number);
    abort();
}
