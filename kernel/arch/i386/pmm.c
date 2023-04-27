#include <stdio.h>
#include <stdlib.h>
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

void pmm_allocate_frame(page_t *page, bool kernel, bool writeable) {
    // If the page already has an address specified, then it is done
    if (page->address != 0) {
        return;
    } else {
        uint32_t index = pmm_first_free_frame();

        // No free frames found
        if (index == (uint32_t)-1) {
            kerror("No free frames!");
            abort();
        }

        // Modify bitmap
        pmm_set_frame(index * 0x1000);

        // Clear and setup the page
        page->present = 1;
        page->read_write = (writeable ? 1 : 0);
        page->user_mode = (kernel ? 0 : 1);
        page->address = index;
    }
}

void pmm_free_frame(page_t *page) {
    // Page is not actually allocated
    if (!page->address) {
        return;
    }

    // Clear both the address and the frame in the bitap
    pmm_clear_frame(page->address);
    page->address = 0;
}

page_t *pmm_get_page(uint32_t address, bool create, page_directory_t *dir) {
    address /= 0x1000;

    uint32_t table_index = address / 1024;

    if (dir->tables[table_index]) {
        return &dir->tables[table_index]->pages[address % 1024];
    } else if (create) {
        uint32_t tmp;

        dir->tables[table_index] = (page_table_t*)kmalloc_physical_aligned(
                sizeof(page_table_t), true, &tmp);
        memset(dir->tables[table_index], 0, 0x1000);
        printf("Table located at: 0x%X\n", dir->tables[table_index]);
        // TODO: Add granularity of control here
        dir->physical_tables[table_index] = tmp | 0x7;

        return &dir->tables[table_index]->pages[address % 1024];
    } else {
        return 0;
    }
}

void pmm_switch_page_directory(page_directory_t *directory) {
    current_directory = directory;
    uint32_t cr0;

    asm volatile("mov %0, %%cr3" :: "r" (&directory->physical_tables));
    asm volatile("mov %%cr0, %0" : "=r" (cr0));
    
    cr0 |= 0x80000000;
    asm volatile("cli; mov %0, %%cr0" :: "r" (cr0));
}

void page_fault(registers_t *regs) {
    kerror("Page fault!");
    abort();
}

// Initialize paging
void pmm_initialize() {
    uint32_t mem_end_page = 0x1000000;

    frame_count = mem_end_page / 0x1000;
    bit_frames = (uint32_t*)kmalloc(INDEX_FROM_BIT(frame_count));
    memset(bit_frames, 0, INDEX_FROM_BIT(frame_count));

    pmm_kernel_directory = (page_directory_t*)kmalloc_aligned(
            sizeof(page_directory_t), true);
    current_directory = pmm_kernel_directory;

    for (uint32_t i = 0; i < placement_address; i += 0x1000) {
        pmm_allocate_frame(pmm_get_page(i, 1, pmm_kernel_directory), 0, 0);
    }

    register_interrupt_handler(14, page_fault);
    pmm_switch_page_directory(pmm_kernel_directory);
}
