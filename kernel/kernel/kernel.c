#include <stdio.h>
#include <stdlib.h>
#include <kernel/tty.h>
#include <kernel/vmm.h>
#include <kernel/multiboot.h>
#include <fs/vfs.h>
#include <arch/i386/descriptor_tables.h>
#include <arch/i386/pmm.h>
#include <string.h>
#include <drivers/ps2_keyboard.h>

volatile uint32_t tick = 0;
void timer_callback(registers_t* regs) {
    tick++;
    printf("Tick: %d\n", tick);
}

extern uint32_t placement_address;

void kernel_main(multiboot_header_t *mboot_ptr) {
    terminal_initialize();
    printf("Hello, world!\n");
    gdt_initialize();
    printf("Initialized GDT\n");
    idt_initialize();
    printf("Initialized ISR\n");

    // At least m1.mod needs to load, this is the initial ramdisk.
    if (mboot_ptr->mods_count <= 0) {
        panic("Aborting due to module #1 failing to load\n");
    }

    // Free memory begins after `mboot_ptr->mods_count` pointers.
    placement_address = (
        *(uint32_t*)(mboot_ptr->mods_addr + 4)
    );

    pmm_initialize();
   
    printf("Initializing devices...\n");
    device_init();
    vfs_init();
    void *initrd_address = *((uint32_t*)mboot_ptr->mods_addr);

    initrd_init(initrd_address);
    printf("\tRAMDisk initialized\n");
    device_printall();

    ps2_keyboard_init();
    
    asm volatile("sti");

    for(;;) {
        asm("hlt");
    }
}
