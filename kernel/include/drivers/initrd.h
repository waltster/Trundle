#ifndef _INITRD_H
#define _INITRD_H 1

#include <stdint.h>

typedef struct {
    void *mem_loc;
    uint32_t length;
} initrd_info_t;

uint32_t initrd_probe(device_t *dev);
uint32_t initrd_read(uint8_t *buffer, uint32_t offset, uint32_t len, 
        device_t *dev);
uint32_t initrd_write(uint8_t *buffer, uint32_t offset, uint32_t len,
        device_t *dev);
uint32_t initrd_read_file(char *name, char *buffer, uint32_t len,
        device_t *dev);
uint32_t initrd_read_dir(char *name, char *buffer, device_t *dev);
uint32_t initrd_write_file(char *name, char *buffer, uint32_t len, 
        device_t *dev);
uint32_t initrd_exists(char *name, device_t *dev);
uint32_t initrd_mount(char *loc, device_t *dev);
uint32_t initrd_unmount(device_t *dev);
void initrd_init(void *mem_loc);

#endif
