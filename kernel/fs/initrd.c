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
#include <fs/vfs.h>
#include <fs/myfs.h>
#include <drivers/initrd.h>

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
               ": %d\n", offset);
        return 0;
    }

    memcpy(buffer, buf_read + offset, length); 
    return length;
}

// Initrd cannot be written to, as it only exists in memory.
uint32_t initrd_write(uint8_t *buffer, uint32_t offset, uint32_t length,
        device_t *dev) {
    return -1;
}

uint32_t initrd_read_file(file_t *file, uint8_t *buffer, uint32_t len, 
        device_t *dev) {
    if (len <= 0) return 0;
    
    uint32_t pos = file->read_pos;

    return initrd_read(buffer, pos, file->file_size, dev);
}

uint32_t initrd_read_dir(file_t *dir, device_t *dev) {
    panic("Unimplemented");
    return -1;
}

file_t *initrd_open_dir(char *name) {
    panic("Unimplemented");
    return NULL;
}

file_t *initrd_open(char *name, device_t *dev) {
    printf("Name: %s\n", name);

    if (!name || !dev) {
        printf("Name or device unset!\n");
        return NULL;
    }

    initrd_info_t *info = (initrd_info_t*)dev->data;
    myfs_header_t *header = (myfs_header_t*)info->mem_loc;

    if (header->magic != MYFS_MAGIC) {
        printf("Magic value does not match!\n");
        return NULL;
    }

    uint8_t *raw = (uint8_t*)info->mem_loc;

    raw += sizeof(myfs_header_t);

    int adjuster;
    int mount_index = vfs_get_mount_point_index(name, &adjuster);

    while (raw < (((uint8_t*)info->mem_loc) + info->length)) {
        myfs_file_header_t *file_header = (myfs_file_header_t*)raw;

        if (strcmp(file_header->file_name, name + 1 + adjuster) == 0) {
            file_t *fp = (file_t*)kmalloc(sizeof(file_t));
            assert(fp != NULL);
            memset(fp, 0, sizeof(file_t));

            fp->is_dir = 0;
            fp->flags = 0;
            fp->mount_index = mount_index;
            fp->read_pos = 0;
            fp->file_pos = 0;
            fp->data = (uint32_t)file_header + sizeof(myfs_file_header_t) 
                - (uint32_t)info->mem_loc;
            fp->file_size = file_header->file_size;
            return fp;
        }

        raw += sizeof(myfs_file_header_t) + file_header->file_size;
    }

    return NULL;
}

uint32_t initrd_write_file(file_t *file, uint8_t *buffer, uint32_t len,
        device_t *dev) {
    return 0;
}

uint32_t initrd_exists(char *name, device_t *dev) {
    return 0;
}

uint32_t initrd_mount(char *loc, device_t *dev) {
    return 1;
}

uint32_t initrd_unmount(device_t *dev) {
    return 0;
}

void initrd_init(void *mem_loc) {
    device_t *dev_initrd = (device_t*)kmalloc(sizeof(device_t));
    assert(dev_initrd != NULL);
    memset(dev_initrd, 0, sizeof(device_t));

    initrd_info_t *initrd_info = (initrd_info_t*)kmalloc(sizeof(initrd_info_t));
    assert(initrd_info != NULL);
    memset(initrd_info, 0, sizeof(initrd_info_t));

    myfs_header_t *myfs_header = (myfs_header_t*)mem_loc;

    initrd_info->mem_loc = mem_loc;
    initrd_info->length = myfs_header->total_length;

    fs_t *fs_initrd = (fs_t*)kmalloc(sizeof(fs_t));
    assert(fs_initrd != NULL);
    memset(fs_initrd, 0, sizeof(fs_t));

    strncpy(dev_initrd->name, "initrd", sizeof(dev_initrd->name));
    dev_initrd->uid = 3;
    dev_initrd->type = CHAR;
    dev_initrd->filesystem = (void*)fs_initrd;
    dev_initrd->read = &initrd_read;
    dev_initrd->write = &initrd_write;
    dev_initrd->data = initrd_info;

    strncpy(fs_initrd->name, "initrd", sizeof(fs_initrd->name));
    fs_initrd->probe = &initrd_probe;
    fs_initrd->read = &initrd_read_file;
    fs_initrd->read_dir = &initrd_read_dir;
    fs_initrd->open = &initrd_open;
    fs_initrd->open_dir = &initrd_open_dir;
    fs_initrd->write = &initrd_write_file;
    fs_initrd->exists = &initrd_exists;
    fs_initrd->mount = &initrd_mount;
    fs_initrd->unmount = &initrd_unmount;

    dev_initrd->filesystem = (void*)fs_initrd;
    
    device_register(dev_initrd);
    dev_initrd = device_get_by_uid(3);
    bool mounted = vfs_mount("/dev/initrd", dev_initrd); 
    
    printf("Attempted mount of '/dev/initrd': %s\n", (mounted ? "success" : "failed"));
}
