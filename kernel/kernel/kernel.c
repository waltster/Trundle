#include <stdio.h>
#include <stdlib.h>
#include <kernel/tty.h>
#include <kernel/vmm.h>
#include <arch/i386/descriptor_tables.h>
#include <arch/i386/pmm.h>

volatile uint32_t tick = 0;
void timer_callback(registers_t* regs) {
    tick++;
    printf("Tick: %d\n", tick);
}

extern uint32_t placement_address;

void kernel_main() {
    terminal_initialize();
    printf("Hello, world!\n");
    gdt_initialize();
    printf("Initialized GDT\n");
    idt_initialize();
    printf("Initialized ISR\n");
    pmm_initialize();
    printf("Initialized PMM\n");
    asm volatile("sti");

    char *test = (char*)kmalloc(10);
   // memcpy(test, "Hello", strlen("Hello"));
    
    printf("0x%X: %s\n", test, test); 
    for(;;) {
        asm("hlt");
    }
}
