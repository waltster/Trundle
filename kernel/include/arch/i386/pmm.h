#ifndef _PMM_H
#define _PMM_H 1

#include <stdint.h>
#include <stdbool.h>
#include <arch/i386/descriptor_tables.h>

#define PMM_ENTRIES_PER_TABLE 1024
#define PMM_TABLES_PER_DIRECTORY 1024
#define INDEX_FROM_BIT(b) (b / (8*4))
#define OFFSET_FROM_BIT(b) (b % (8*4))
#define PAGE_SIZE 4096
#define OUT_OF_MEMORY ((uint32_t)-1)

typedef struct {
    uint32_t present: 1;
    uint32_t read_write: 1;
    uint32_t user_mode: 1;
    uint32_t accessed: 1;
    uint32_t dirty: 1;
    uint32_t unused: 7;
    uint32_t address: 20;
} page_t;

typedef struct {
    page_t pages[PMM_ENTRIES_PER_TABLE];
} page_table_t;

typedef struct {
    page_table_t *tables[PMM_TABLES_PER_DIRECTORY];
    uint32_t physical_tables[PMM_TABLES_PER_DIRECTORY];
    uint32_t physical_address_of_physical_tables;
} page_directory_t;

void page_fault(registers_t *regs);
void pmm_initialize();

/*
 * Allocate a given number of virtually contiguous pages
 *
 * @param pages the number of pages to allocate
 * @return the address of the first page, or OUT_OF_MEMORY if it is impossible.
 */
uint32_t pmm_allocate_pages(int pages);
uint32_t pmm_allocate_and_map_for_heap(size_t pages);
void pmm_free_page(uint32_t virtual_address);
void pmm_free_pages(uint32_t virtual_address, size_t pages);
void pmm_unmap_page(page_directory_t *dir, uint32_t virtual_address);
page_t *pmm_get_page(page_directory_t *dir, uint32_t virtual_address, 
        bool create);
void pmm_map_page(page_directory_t *dir, uint32_t virtual, uint32_t physical, 
        bool writeable, bool user_mode);

#endif
