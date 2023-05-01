#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <arch/i386/pmm.h>
#include <arch/i386/descriptor_tables.h>
#include <kernel/vmm.h>

page_directory_t *pmm_kernel_directory = 0;
page_directory_t *current_directory = 0;
extern uint32_t placement_address;
uint32_t *bit_frames;
uint32_t frame_count;

void pmm_set_frame(uint32_t frame_address) {
    uint32_t frame = frame_address / 0x1000;
    uint32_t index = INDEX_FROM_BIT(frame);
    uint32_t offset = OFFSET_FROM_BIT(frame);

    bit_frames[index] |= (0x1 << offset);
}

void pmm_clear_frame(uint32_t frame_address) {
    uint32_t frame = frame_address / 0x1000;
    uint32_t index = INDEX_FROM_BIT(frame);
    uint32_t offset = OFFSET_FROM_BIT(frame);

    bit_frames[index] &= ~(1 << offset);
}

bool pmm_test_frame(uint32_t frame_address) {
    uint32_t frame = frame_address / 0x1000;
    uint32_t index = INDEX_FROM_BIT(frame);
    uint32_t offset = OFFSET_FROM_BIT(frame);

    return (bit_frames[index] & (0x1 << offset)) ? true : false;
}

void pmm_flush(uint32_t virtual_address) {
    asm volatile("invlpg (%0)" :: "a" (virtual_address));
}

uint32_t pmm_first_free_frame() {
    for (uint32_t i = 0; i < INDEX_FROM_BIT(frame_count); i++) {
        if (bit_frames[i] != 0xFFFFFFFF) {
            for (uint32_t j = 0; j < 32; j++) {
                uint32_t bit_to_test = 0x1 << j;

                if (!(bit_frames[i] & bit_to_test)) {
                    return i * 32 + j;
                }
            }
        }
    }

    return -1;
}

void page_fault(registers_t *regs) {
    uint32_t faulting_address;
    asm volatile("mov %%cr2, %0" : "=r" (faulting_address));

    int present = !(regs->error_number & 0b1);
    int rw = regs->error_number & 0b10;
    int user = regs->error_number & 0b100;
    int reserved = regs->error_number & 0b1000;
    int id = regs->error_number & 0b10000;

    printf("Page fault!\n");
    if (present) printf("\tNot present\n");
    if (rw) printf("\tRead-only\n");
    if (user) printf("\tUser mode\n");
    if (reserved) printf("\tReserved\n");
    printf("At 0x%X\n", faulting_address);

    abort();
}

void pmm_switch_page_directory(page_directory_t *directory) {
    current_directory = directory;
    uint32_t cr0;

    asm volatile("mov %0, %%cr3" :: "r" (&directory->physical_tables));
    asm volatile("mov %%cr0, %0" : "=r" (cr0));
    
    cr0 |= 0x80000000;
    asm volatile("cli; mov %0, %%cr0" :: "r" (cr0));
}

uint32_t pmm_allocate_pages(int pages) {
    // TODO: Support multiple pages
    if (pages > 1) {
        panic("Cannot allocate multiple pages right now");
    }

    int index = 0;

    for (uint32_t index = 0; index < INDEX_FROM_BIT(frame_count); index++) {
        // Bitframe is full
        if (bit_frames[index] != 0xFFFFFFFF) {
            for (uint32_t bit = 0; bit < 32; bit++) {
                uint32_t to_test = 1 << bit;

                if (!(bit_frames[index] & to_test)) {
                    uint32_t addr = (index * 32) + bit;

                    pmm_set_frame(addr * 0x1000);
                    return (addr) * 0x1000;
                }
            }
        }

    }

    return OUT_OF_MEMORY;
}

page_t *pmm_get_page(page_directory_t *dir, uint32_t virtual_address, 
        bool create) {
    virtual_address /= 0x1000;

    uint32_t table_index = virtual_address / 1024;

    if (dir->tables[table_index]) {
        return &dir->tables[table_index]->pages[virtual_address % 1024];
    } else if (create) {
        uint32_t tmp;

        dir->tables[table_index] = (page_table_t*)kmalloc_physical_aligned(
                sizeof(page_table_t), true, &tmp);
        memset(dir->tables[table_index], 0, 0x1000);

        dir->physical_tables[table_index] = tmp | 0x7;

        return &dir->tables[table_index]->pages[virtual_address % 1024];
    } else {
        abort();
        return 0;
    }
}

void pmm_map_page(page_directory_t *dir, uint32_t virtual, uint32_t physical,
        bool writeable, bool user_mode) {
    page_t *page = pmm_get_page(dir, virtual, true);
    page->address = physical / 0x1000;
    page->read_write = (writeable ? 1 : 0);
    page->user_mode = (user_mode ? 0 : 1);
    page->present = 1;
}

void pmm_initialize() {
    uint32_t mem_end_page = 0x1000000;
    frame_count = mem_end_page / PAGE_SIZE;

    bit_frames = (uint32_t*)kmalloc(INDEX_FROM_BIT(frame_count));
    pmm_kernel_directory = (page_directory_t*)kmalloc_aligned(
           sizeof(page_directory_t), true);

    memset(bit_frames, 0, INDEX_FROM_BIT(frame_count));
    memset(pmm_kernel_directory, 0, sizeof(page_directory_t)); 

    current_directory = pmm_kernel_directory;

    for (uint32_t i = HEAP_START; i < HEAP_START + HEAP_INITIAL_SIZE;
            i += PAGE_SIZE) {
        pmm_get_page(pmm_kernel_directory, i, true);
    }

    for (uint32_t i = 0; i < placement_address + 0x1000; i += 0x1000) {
        uint32_t phys = pmm_allocate_pages(1);
        uint32_t virt = i;

        pmm_map_page(pmm_kernel_directory, virt, phys, false, false);
    }

    for (uint32_t i = HEAP_START; i < HEAP_START + HEAP_INITIAL_SIZE;
            i += PAGE_SIZE) {
        uint32_t phys = pmm_allocate_pages(1);
        uint32_t virt = i;

        pmm_map_page(pmm_kernel_directory, virt, phys, true, false);
    }

    register_interrupt_handler(14, page_fault);
    pmm_switch_page_directory(pmm_kernel_directory);

    printf("Done!\n");
    heap_initialize(HEAP_START, HEAP_INITIAL_SIZE);
}
