#include <stdio.h>
#include <stdlib.h>
#include <kernel/tty.h>
#include <arch/i386/gdt.h>

void kernel_main() {
    terminal_initialize();
    printf("Hello, world!\n");
    gdt_initialize();
    printf("Initialized GDT\n");

    abort();
}
