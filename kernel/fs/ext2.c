#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <kernel/vmm.h>
#include <fs/ext2.h>
#include <fs/vfs.h>

uint32_t ext2_probe(device_t *dev) { 
    uint8_t *buffer = (uint8_t*)kmalloc(1024);
    ext2_superblock_t *superblock;
    fs_t *filesystem;

    memset(buffer, 0, 1024);
    dev->read(buffer, 0, 1024, dev);
   
    superblock = (ext2_superblock_t*)buffer;

    if (superblock->signature != EXT2_SIGNATURE) {
        printf("Failed to verify signature for EXT2\n");
        return 0;
    }

    uint32_t blocksize = EXT2_CONVERT_SIZE(superblock->block_size);

    printf("Block size: %d\n", blocksize);

    kfree(buffer);   
}

uint32_t ext2_mount(char *loc, device_t *dev) {

}
