/*
 * The initial ramdisk (initrd) is a special kind of filesystem. Normally, the
 * stack of drivers goes something like this:
 *      hardware device driver -> memory device driver -> vfs driver -> libc
 * however, in the case of the initrd, the hardware device driver and memory
 * device driver are all wrapped into one. This is because the "hardware" is
 * really just a section of kernel memory. It can be abstracted away with a
 * global address.
 *
 * Implementation of future filesystems will require that the one calling
 * `initialize` will provide the `device_t` structure, complete with its read
 * and write calls. For simplicity, it is all implemented here, but future
 * additions will require such attention.
 * - W
 */
#include <stdio.h>
#include <string.h>
#include <kernel/vmm.h>
#include <kernel/vfs.h>
#include <drivers/initrd.h>
#include <drivers/myfs.h>

uint32_t initrd_probe(device_t *dev) {
    myfs_header_t myfs_header;

    if (initrd_read((uint8_t*)&myfs_header, 0, sizeof(myfs_header_t), dev) < 
            sizeof(myfs_header_t)) {
        return 0;
    }

    if (myfs_header.magic == MYFS_MAGIC) return 1;

    return 0;
}

// Simply copies from initrd into the buffer, respecting the boundary.
uint32_t initrd_read(uint8_t *buffer, uint32_t offset, uint32_t length, 
        device_t *dev) {
    initrd_info_t *info = (initrd_info_t*)dev->data;
    uint8_t *buf_read = (uint8_t*)info->mem_loc;

    if ((int)offset < 0 || (int)(offset + length) < 0) {
        printf("Warning: attempted to read at offset smaller than 0 on initrd"
               "\n");
        return 0;
    }

    if (offset + length >= info->length) {
        printf("Warning: attempted to read more bytes than exist on initrd, "
               "adjusting length.\n");
        length = info->length - offset;
    }

    memcpy(buffer, buf_read + offset, length); 
    return length;
}

// Initrd cannot be written to, as it only exists in memory.
uint32_t initrd_write(uint8_t *buffer, uint32_t offset, uint32_t length,
        device_t *dev) {
    return -1;
}

uint32_t initrd_read_file(char *name, char *buffer, uint32_t len, 
        device_t *dev) {
    return -1;
}

uint32_t initrd_read_dir(char *name, char *buffer, device_t *dev) {
    return -1;
}

uint32_t initrd_write_file(char *name, char *buffer, uint32_t len,
        device_t *dev) {
    return -1;
}

uint32_t initrd_exists(char *name, device_t *dev) {
    return -1;
}

uint32_t initrd_mount(char *loc, device_t *dev) {
    return -1;
}

uint32_t initrd_unmount(device_t *dev) {
    return -1;
}

void initrd_init(void *mem_loc) {
    device_t *dev_initrd = (device_t*)kmalloc(sizeof(device_t));
    assert(dev_initrd != NULL);
    memset(dev_initrd, 0, sizeof(device_t));

    fs_t *fs_initrd = (fs_t*)kmalloc(sizeof(fs_t));
    assert(fs_initrd != NULL);
    memset(fs_initrd, 0, sizeof(fs_t));

    memcpy(dev_initrd->name, "initrd", strlen("initrd"));
    dev_initrd->uid = 3;
    dev_initrd->type = CHAR;
    dev_initrd->filesystem = (void*)fs_initrd;
    dev_initrd->read = &initrd_read;
    dev_initrd->write = &initrd_write;
    dev_initrd->data = mem_loc;

    memcpy(fs_initrd->name, "initrd", strlen("initrd"));
    fs_initrd->probe = &initrd_probe;
    fs_initrd->read = &initrd_read_file;
    fs_initrd->read_dir = &initrd_read_dir;
    fs_initrd->write = &initrd_write_file;
    fs_initrd->exists = &initrd_exists;
    fs_initrd->mount = &initrd_mount;
    fs_initrd->unmount = &initrd_unmount;

    device_register(dev_initrd);
    dev_initrd = device_get_by_uid(3);
    bool mounted = vfs_mount("/dev/initrd", dev_initrd); 
    
    printf("Attempted mount of '/dev/initrd': %s\n", (mounted ? "success" : "failed"));
}
