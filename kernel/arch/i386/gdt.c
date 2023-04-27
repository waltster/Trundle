#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <arch/i386/descriptor_tables.h>
#include <arch/i386/ports.h>

gdt_ptr_t gdt_ptr;
gdt_entry_t gdt_entries[GDT_MAX_ENTRIES];
idt_ptr_t idt_ptr;
idt_entry_t idt_entries[IDT_MAX_ENTRIES];
isr_t interrupt_handlers[256];

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
    memset(&interrupt_handlers, 0, sizeof(isr_t) * 256);

    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    outb(0x21, 0x20);
    outb(0xA1, 0x28);
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    outb(0x21, 0x0);
    outb(0xA1, 0x0);

    idt_make_entry(0, (uint32_t)&isr_0, 0x08, IDT_TRAP_GATE);
    idt_make_entry(1, (uint32_t)&isr_1, 0x08, IDT_TRAP_GATE);
    idt_make_entry(2, (uint32_t)&isr_2, 0x08, IDT_TRAP_GATE);
    idt_make_entry(3, (uint32_t)&isr_3, 0x08, IDT_TRAP_GATE);
    idt_make_entry(4, (uint32_t)&isr_4, 0x08, IDT_TRAP_GATE);
    idt_make_entry(5, (uint32_t)&isr_5, 0x08, IDT_TRAP_GATE);
    idt_make_entry(6, (uint32_t)&isr_6, 0x08, IDT_TRAP_GATE);
    idt_make_entry(7, (uint32_t)&isr_7, 0x08, IDT_TRAP_GATE);
    idt_make_entry(8, (uint32_t)&isr_8, 0x08, IDT_TRAP_GATE);
    idt_make_entry(9, (uint32_t)&isr_9, 0x08, IDT_TRAP_GATE);
    idt_make_entry(10, (uint32_t)&isr_10, 0x08, IDT_TRAP_GATE);
    idt_make_entry(11, (uint32_t)&isr_11, 0x08, IDT_TRAP_GATE);
    idt_make_entry(12, (uint32_t)&isr_12, 0x08, IDT_TRAP_GATE);
    idt_make_entry(13, (uint32_t)&isr_13, 0x08, IDT_TRAP_GATE);
    idt_make_entry(14, (uint32_t)&isr_14, 0x08, IDT_TRAP_GATE);
    idt_make_entry(15, (uint32_t)&isr_15, 0x08, IDT_TRAP_GATE);
    idt_make_entry(16, (uint32_t)&isr_16, 0x08, IDT_TRAP_GATE);
    idt_make_entry(17, (uint32_t)&isr_17, 0x08, IDT_TRAP_GATE);
    idt_make_entry(18, (uint32_t)&isr_18, 0x08, IDT_TRAP_GATE);
    idt_make_entry(19, (uint32_t)&isr_19, 0x08, IDT_TRAP_GATE);
    idt_make_entry(20, (uint32_t)&isr_20, 0x08, IDT_TRAP_GATE);
    idt_make_entry(21, (uint32_t)&isr_21, 0x08, IDT_TRAP_GATE);
    idt_make_entry(22, (uint32_t)&isr_22, 0x08, IDT_TRAP_GATE);
    idt_make_entry(23, (uint32_t)&isr_23, 0x08, IDT_TRAP_GATE);
    idt_make_entry(24, (uint32_t)&isr_24, 0x08, IDT_TRAP_GATE);
    idt_make_entry(25, (uint32_t)&isr_25, 0x08, IDT_TRAP_GATE);
    idt_make_entry(26, (uint32_t)&isr_26, 0x08, IDT_TRAP_GATE);
    idt_make_entry(27, (uint32_t)&isr_27, 0x08, IDT_TRAP_GATE);
    idt_make_entry(28, (uint32_t)&isr_28, 0x08, IDT_TRAP_GATE);
    idt_make_entry(29, (uint32_t)&isr_29, 0x08, IDT_TRAP_GATE);
    idt_make_entry(30, (uint32_t)&isr_30, 0x08, IDT_TRAP_GATE);
    idt_make_entry(31, (uint32_t)&isr_31, 0x08, IDT_TRAP_GATE);
    idt_make_entry(32, (uint32_t)&irq_0, 0x08, IDT_INTERRUPT_GATE);
    idt_make_entry(33, (uint32_t)&irq_1, 0x08, IDT_INTERRUPT_GATE);
    idt_make_entry(34, (uint32_t)&irq_2, 0x08, IDT_INTERRUPT_GATE);
    idt_make_entry(35, (uint32_t)&irq_3, 0x08, IDT_INTERRUPT_GATE);
    idt_make_entry(36, (uint32_t)&irq_4, 0x08, IDT_INTERRUPT_GATE);
    idt_make_entry(37, (uint32_t)&irq_5, 0x08, IDT_INTERRUPT_GATE);
    idt_make_entry(38, (uint32_t)&irq_6, 0x08, IDT_INTERRUPT_GATE);
    idt_make_entry(39, (uint32_t)&irq_7, 0x08, IDT_INTERRUPT_GATE);
    idt_make_entry(40, (uint32_t)&irq_8, 0x08, IDT_INTERRUPT_GATE);
    idt_make_entry(41, (uint32_t)&irq_9, 0x08, IDT_INTERRUPT_GATE);
    idt_make_entry(42, (uint32_t)&irq_10, 0x08, IDT_INTERRUPT_GATE);
    idt_make_entry(43, (uint32_t)&irq_11, 0x08, IDT_INTERRUPT_GATE);
    idt_make_entry(44, (uint32_t)&irq_12, 0x08, IDT_INTERRUPT_GATE);
    idt_make_entry(45, (uint32_t)&irq_13, 0x08, IDT_INTERRUPT_GATE);
    idt_make_entry(46, (uint32_t)&irq_14, 0x08, IDT_INTERRUPT_GATE);
    idt_make_entry(47, (uint32_t)&irq_15, 0x08, IDT_INTERRUPT_GATE);

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
    if (interrupt_handlers[regs->interrupt_number] != 0) {
        isr_t handler = interrupt_handlers[regs->interrupt_number];
        handler(regs);
    } else {
        printf("Unhandled interrupt: %d\n", regs->interrupt_number);
        abort();
    }
}

void irq_handler(registers_t* regs) {
    if (regs->interrupt_number >= 40) {
        outb(0xA0, 0x20);
    }

    outb(0x20, 0x20);
    
    if (interrupt_handlers[regs->interrupt_number] != 0) {
        isr_t handler = interrupt_handlers[regs->interrupt_number];
        handler(regs);
    }
}

void register_interrupt_handler(uint8_t index, isr_t handler) {
    printf("Registering interrupt %d\n", index);
    interrupt_handlers[index] = handler;
}
