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
extern uint32_t heap_location;

uint32_t *bit_frames;
uint32_t frame_count;

static inline void pmm_set_frame(uint32_t frame_address) {
    uint32_t frame = frame_address / PAGE_SIZE;
    uint32_t index = INDEX_FROM_BIT(frame);
    uint32_t offset = OFFSET_FROM_BIT(frame);

    bit_frames[index] |= (0x1 << offset);
}

static inline void pmm_clear_frame(uint32_t frame_address) {
    uint32_t frame = frame_address / PAGE_SIZE;
    uint32_t index = INDEX_FROM_BIT(frame);
    uint32_t offset = OFFSET_FROM_BIT(frame);

    bit_frames[index] &= ~(1 << offset);
}

static bool pmm_test_frame(uint32_t frame_address) {
    uint32_t frame = frame_address / PAGE_SIZE;
    uint32_t index = INDEX_FROM_BIT(frame);
    uint32_t offset = OFFSET_FROM_BIT(frame);

    return (bit_frames[index] & (0x1 << offset)) ? true : false;
}

/**
 * Invalidate the page table for a given address
 *
 * @param virtual_address the virtual address of the page to invalidate
 */
static void pmm_flush(uint32_t virtual_address) {
    asm volatile("invlpg (%0)" :: "a" (virtual_address));
}

/**
 * Find the physical address of the first free frame. 
 *
 * This does not promise any kind of contiguity, just that the frame is
 * currently unused.
 *
 * @return the first free frame or OUT_OF_MEMORY
 */
static uint32_t pmm_first_free_frame() {
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

    return OUT_OF_MEMORY;
}

void page_fault(registers_t *regs) {
    uint32_t faulting_address;
    asm volatile("mov %%cr2, %0" : "=r" (faulting_address));

    int present = !(regs->error_number & 0b1);
    int rw = regs->error_number & 0b10;
    int user = regs->error_number & 0b100;
    int reserved = regs->error_number & 0b1000;

    printf("Page fault!\n");
    if (present) printf("\tNot present\n");
    if (rw) printf("\tRead-only\n");
    if (user) printf("\tUser mode\n");
    if (reserved) printf("\tReserved\n");
    printf("At 0x%X\n", faulting_address);

    abort();
}

static void pmm_switch_page_directory(page_directory_t *directory) {
    current_directory = directory;
    uint32_t cr0;

    asm volatile("mov %0, %%cr3" :: "r" (&directory->physical_tables));
    asm volatile("mov %%cr0, %0" : "=r" (cr0));
    
    cr0 |= 0x80000000;
    asm volatile("cli; mov %0, %%cr0" :: "r" (cr0));
}

/**
 * Allocate a given number of virtually contiguous pages
 *
 * @param pages the number of pages to allocate
 * @return the address of the first page, or OUT_OF_MEMORY if it is impossible.
 */
uint32_t pmm_allocate_pages(int pages) {
    if (pages <= 0) return OUT_OF_MEMORY;

    uint32_t index = 0;
    uint32_t free_frames = 0;

    while (index < INDEX_FROM_BIT(frame_count)) {
        // Check if the frame is full
        if (bit_frames[index] != 0xFFFFFFFF) {
            // Loop over every frame
            for (uint32_t bit = 0; bit < 32; bit++) {
                uint32_t to_test = 1 << bit;

                // If this is a free frame in the map
                if (!(bit_frames[index] & to_test)) {
                    free_frames++;

                    // Did we find all the frames we need to?
                    if (free_frames == pages) {
                        // Get the first frame we found as free
                        uint32_t first_free_frame = (index * 32 + bit) - pages + 1;
                        uint32_t first_addr = first_free_frame * PAGE_SIZE;

                        index = 0;
                        while (index < (uint32_t)pages) {
                            uint32_t current_frame_index = (first_free_frame + index);
                            uint32_t current_addr = current_frame_index * PAGE_SIZE;

                            pmm_set_frame(current_addr);
                            index++;
                        }
                        return first_addr;
                    }
                }
                // Otherwise, it is occupied, we need to move on
                else {
                    free_frames = 0;
                }
            }
        }

        index++;
    }

/* old:
    for (uint32_t index = 0; index < INDEX_FROM_BIT(frame_count); index++) {
        // Bitframe is full
        if (bit_frames[index] != 0xFFFFFFFF) {
            for (uint32_t bit = 0; bit < 32; bit++) {
                uint32_t to_test = 1 << bit;

                if (!(bit_frames[index] & to_test)) {
                    uint32_t addr = (index * 32) + bit;

                    pmm_set_frame(addr * PAGE_SIZE);
                    return (addr * PAGE_SIZE);
                }
            }
        }

    }*/

    return OUT_OF_MEMORY;
}

uint32_t pmm_allocate_and_map_for_heap(size_t pages) {
    printf("Call to %s\n", __FUNCTION__);

    uint32_t physicals = pmm_allocate_pages(pages);

    if (physicals == OUT_OF_MEMORY) {
        return OUT_OF_MEMORY;
    }

    uint32_t new_heap_end = (uint32_t)(heap_location + (pages * PAGE_SIZE));

    if (new_heap_end > HEAP_START + HEAP_INITIAL_SIZE) {
       panic("Cannot allocate more than the initial size for the heap");
    }

    for (uint32_t i = heap_location, j = 0; i < new_heap_end; i += PAGE_SIZE, 
            j += PAGE_SIZE) {
        pmm_map_page(pmm_kernel_directory, i, physicals + j, true, false);
    }

    heap_location = new_heap_end;

    return new_heap_end - (pages * PAGE_SIZE);
}

void pmm_free_page(uint32_t virtual_address) {
    pmm_unmap_page(pmm_kernel_directory, virtual_address);
}

void pmm_free_pages(uint32_t virtual_address, size_t pages) {
    for (int i = virtual_address; i < (int)(virtual_address + (pages * PAGE_SIZE));
            i++) {
        pmm_free_page(i);
    }
}

void pmm_unmap_page(page_directory_t *dir, uint32_t virtual_address) {
    page_t *page = pmm_get_page(dir, virtual_address, false);

    if (!page) return;

    pmm_clear_frame(page->address);
    *((uint32_t*)page) = 0;

    asm volatile("invlpg (%0)" :: "a" (virtual_address));
}

page_t *pmm_get_page(page_directory_t *dir, uint32_t virtual_address, 
        bool create) {
    virtual_address /= PAGE_SIZE;

    uint32_t table_index = virtual_address / 1024;

    if (dir->tables[table_index]) {
        return &dir->tables[table_index]->pages[virtual_address % 1024];
    } else if (create) {
        uint32_t tmp;

        dir->tables[table_index] = (page_table_t*)_kmalloc_physical_aligned(
                sizeof(page_table_t), true, &tmp);
        memset(dir->tables[table_index], 0, PAGE_SIZE);

        dir->physical_tables[table_index] = tmp | 0x7;

        return &dir->tables[table_index]->pages[virtual_address % 1024];
    } else {
        return 0;
    }
}

void pmm_map_page(page_directory_t *dir, uint32_t virtual, uint32_t physical,
        bool writeable, bool user_mode) {
    // Create page in `dir` for address `virtual`
    page_t *page = pmm_get_page(dir, virtual, true);

    page->address = physical / PAGE_SIZE;
    page->read_write = (writeable ? 1 : 0);
    page->user_mode = (user_mode ? 0 : 1);
    page->present = 1;
}

/**
 * Initializes paging:
 *   1. Creates bitmap for storing frame allocations
 *   2. Creates pages for the heap
 *   3. Identity-maps the kernel and one page above
 *      Note: The extra page is for any allocations/accesses done between when
 *      paging is enabled and when the heap is actually initialized. Until
 *      then, every allocation still relies on the `placement_address` way.
 *   4. Identity map the heap at 0xC0000000
 *   5. Register the page fault handler
 *   6. Switch to the kernel directory
 *   7. Initialize the heap
 */
void pmm_initialize() {
    uint32_t mem_end_page = 0x1000000;
    frame_count = mem_end_page / PAGE_SIZE;

    bit_frames = (uint32_t*)_kmalloc(INDEX_FROM_BIT(frame_count));
    pmm_kernel_directory = (page_directory_t*)_kmalloc_aligned(
           sizeof(page_directory_t), true);
    current_directory = pmm_kernel_directory;

    memset(bit_frames, 0, INDEX_FROM_BIT(frame_count));
    memset(pmm_kernel_directory, 0, sizeof(page_directory_t)); 

    /*printf ("Getting heap\n");
    for (uint32_t i = HEAP_START; i < HEAP_START + HEAP_INITIAL_SIZE;
            i += PAGE_SIZE) {
        pmm_get_page(pmm_kernel_directory, i, true);
    }*/

    printf("Identity mapping kernel\n");
    for (uint32_t i = 0; i < placement_address + PAGE_SIZE; i += PAGE_SIZE) {
        uint32_t phys = pmm_allocate_pages(1);
        uint32_t virt = i;

        pmm_map_page(pmm_kernel_directory, virt, phys, false, false);
    }

    // Identity map the heap
    printf("Identity mapping the heap\n");
    for (uint32_t i = HEAP_START; i < HEAP_START + HEAP_INITIAL_SIZE;
            i += PAGE_SIZE) {
        uint32_t phys = pmm_allocate_pages(1);
        uint32_t virt = i;

        if (phys == OUT_OF_MEMORY) panic("Out of memory!");

        pmm_map_page(pmm_kernel_directory, virt, phys, true, false);
    }

    register_interrupt_handler(14, &page_fault);
    pmm_switch_page_directory(pmm_kernel_directory);
}
