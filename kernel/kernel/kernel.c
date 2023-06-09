#include <stdio.h>
#include <stdlib.h>
#include <kernel/tty.h>
#include <kernel/vmm.h>
#include <arch/i386/descriptor_tables.h>
#include <arch/i386/pmm.h>
#include <string.h>

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

    char *test = kmalloc(6);
    memcpy(test, "Hello\0", 6);
    printf("0x%X: %s\n", test, test);

    char *test2 = kmalloc(6);
    memcpy(test2, "Hi   \0", 6);
    printf("Test 2: 0x%X: %s\n", test2, test2);

    kfree(test2);
    char *test3 = kmalloc(6);
    memcpy(test3, "Hi   \0", 6);
    printf("Test 3: 0x%X: %s\n", test3, test3);


    for(;;) {
        asm("hlt");
    }
}
