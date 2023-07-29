#ifndef _GDT_H
#define _GDT_H 1

#include <stdint.h>

#define GDT_MAX_ENTRIES 5
#define IDT_MAX_ENTRIES 256
#define MEMORY_MAX 0xFFFFFFFF

#define GATE_TASK_GATE 0x5
#define GATE_INTERRUPT_16_GATE 0x6
#define GATE_TRAP_16_GATE 0x7
#define GATE_INTERRUPT_32_GATE 0xE
#define GATE_TRAP_32_GATE 0xF

#define IDT_INTERRUPT_GATE 0x8E
#define IDT_TRAP_GATE 0x8F
#define IDT_TASK_GATE 0x85

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

typedef struct {
    uint16_t offset_low;
    uint16_t segment_selector;
    uint8_t reserved;
    uint8_t flags;
    uint16_t offset_high;
} __attribute__((packed)) idt_entry_t;

typedef struct {
    uint16_t size;
    uint32_t offset;
} __attribute__((packed)) idt_ptr_t;

typedef struct {
    uint32_t ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t interrupt_number;
    uint32_t error_number;
    uint32_t eip, cs, eflags, user_sp, ss;
} registers_t;

typedef void (*isr_t)(registers_t*);

void gdt_initialize();
void gdt_make_entry(int index, uint32_t base, uint32_t limit, uint8_t access, 
        uint8_t flags);
extern void gdt_flush(void *ptr);

void idt_initialize();
void idt_make_entry(int index, uint32_t base, uint16_t selector, 
        uint8_t flags);
extern void idt_flush(void *ptr);
void isr_handler(registers_t* regs);
void register_interrupt_handler(uint8_t index, isr_t handler);

extern void isr_0();
extern void isr_1();
extern void isr_2();
extern void isr_3();
extern void isr_4();
extern void isr_5();
extern void isr_6();
extern void isr_7();
extern void isr_8();
extern void isr_9();
extern void isr_10();
extern void isr_11();
extern void isr_12();
extern void isr_13();
extern void isr_14();
extern void isr_15();
extern void isr_16();
extern void isr_17();
extern void isr_18();
extern void isr_19();
extern void isr_20();
extern void isr_21();
extern void isr_22();
extern void isr_23();
extern void isr_24();
extern void isr_25();
extern void isr_26();
extern void isr_27();
extern void isr_28();
extern void isr_29();
extern void isr_30();
extern void isr_31();
extern void irq_0();
extern void irq_1();
extern void irq_2();
extern void irq_3();
extern void irq_4();
extern void irq_5();
extern void irq_6();
extern void irq_7();
extern void irq_8();
extern void irq_9();
extern void irq_10();
extern void irq_11();
extern void irq_12();
extern void irq_13();
extern void irq_14();
extern void irq_15();
extern void irq_16();
extern void irq_17();
extern void irq_18();
extern void irq_19();
extern void irq_20();
extern void irq_21();
extern void irq_22();
extern void irq_23();
extern void irq_24();
extern void irq_25();
extern void irq_26();
extern void irq_27();
extern void irq_28();
extern void irq_29();
extern void irq_30();
extern void irq_31();
extern void irq_32();
extern void irq_33();
extern void irq_34();
extern void irq_35();
extern void irq_36();
extern void irq_37();
extern void irq_38();
extern void irq_39();
extern void irq_40();
extern void irq_41();
extern void irq_42();
extern void irq_43();
extern void irq_44();
extern void irq_45();
extern void irq_46();
extern void irq_47();

#endif
