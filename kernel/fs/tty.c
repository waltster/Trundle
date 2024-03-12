/*
 * This file implements the tty which is mounted on /dev/tty.
 * Input is buffered here for stdin and stdout as a file.
 */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <kernel/vfs.h>

#define FD_STDIN 0
#define FD_STDOUT 1
#define FD_STDERR 3

#define DEFAULT_BUFFER_SIZE 1024

// Kernel file descriptors
// These should eventually be created and attached to the main process
file_t *k_stdin;
file_t *k_stdout;
file_t *k_stderr;

static uint32_t pos_input_buffer = 0;
static uint32_t pos_output_buffer = 0;
static uint8_t *input_buffer;
static uint8_t *output_buffer;

uint32_t tty_read_stdin(uint8_t *buffer, uint32_t offset, uint32_t len, 
        device_t *dev) {
    // Circular buffer
    offset = offset % DEFAULT_BUFFER_SIZE;

    uint32_t index = offset;

    if (offset + len < DEFAULT_BUFFER_SIZE) {
        memcpy(buffer + offset, input_buffer, len);
    } else {
        // Read to the end of the buffer
        memcpy(buffer + offset, input_buffer, DEFAULT_BUFFER_SIZE - (offset));
        // Read the remaining buffer
        memcpy(buffer, input_buffer, len - (DEFAULT_BUFFER_SIZE - (offset)));
    }

    return len;
}

uint32_t tty_read_file(file_t *file, uint8_t *buffer, uint32_t len,
        device_t *dev) {
    if (len <= 0) return 0;

    if (file == k_stdin) {
        // By using file->file_pos as the offset, it allows seeking
        uint32_t bytes_read = tty_read_stdin(buffer, file->file_pos, len, dev);
        file->file_pos = (file->file_pos + len) % DEFAULT_BUFFER_SIZE;

        return bytes_read;
    } else if (file == k_stdout) {
        panic("Unimplemented");
        return 0;
    } else if (file == k_stderr) {
        panic("Unimplemented");
        return 0;
    }
}

uint32_t tty_write_stdin(uint8_t *buffer, uint32_t offset, uint32_t len,
        device_t *dev) {
    offset = offset % DEFAULT_BUFFER_SIZE;

    uint32_t index = offset;
    
    if (offset + len < DEFAULT_BUFFER_SIZE) {
        memcpy(input_buffer + offset, buffer, len);
    } else {
        memcpy(input_buffer + offset, buffer, DEFAULT_BUFFER_SIZE - offset);
        memcpy(input_buffer, buffer, len - (DEFAULT_BUFFER_SIZE - offset));
    }
}

uint32_t tty_write_stub(uint8_t *buffer, uint32_t offset, uint32_t len,
        device_t *dev){
    panic("Direct call to filesystem write unimplemented.");
    return 0;
}

uint32_t tty_read_stub(uint8_t *buffer, uint32_t offset, uint32_t len,
        device_t *dev){
    panic("Direct call to filesystem read unimplemented.");
    return 0;
}

uint32_t tty_write_file(file_t *file, uint8_t *buffer, uint32_t len,
        device_t *dev) {
    if (len <= 0) return 0;

    if (file == k_stdin) {
        uint32_t bytes_written = tty_write_stdin(buffer, pos_input_buffer, len, dev);
        pos_input_buffer = (pos_input_buffer + len) % DEFAULT_BUFFER_SIZE;

        return bytes_written;
    } else if (file == k_stdout) {
        return tty_write();
    } else if (file == k_stderr) {
        panic("Unimplemented");
        return 0;
    }  
}

// Return true if the tty has not been mounted, otherwise false.
uint32_t tty_probe(device_t *dev) {
    return vfs_check_mount("/dev/tty") ? 0 : 1;
}

uint32_t tty_return_false() {
    return 0;
}

file_t *tty_open(char *name, device_t *dev) {
    if (!name || !dev) {
        printf("TTY: file name or devic unset!\n");
        return NULL;
    }

    if (strcmp(name, "stdin") == 0) {
        return k_stdin;
    } else if (strcmp(name, "stdout") == 0) {
        return k_stdout;
    } else if (strcmp(name, "stderr") == 0) {
        return k_stderr;
    }

    return NULL;
}

uint32_t tty_exists(char *name, device_t *dev) {
    if (!name || !dev) {
        printf("TTY: file name or devic unset!\n");
        return NULL;
    }

    if (strcmp(name, "stdin") == 0) {
        return 1;
    } else if (strcmp(name, "stdout") == 0) {
        return 1;
    } else if (strcmp(name, "stderr") == 0) {
        return 1;
    }

    return 0;
}

uint32_t tty_mount(char *loc, device_t *dev) {
    return 1;
}

uint32_t tty_unmount(device_t *dev) {
    return 0;
}

// Called by kernl
void tty_init() {
    device_t *dev_tty = (device_t*)kmalloc(sizeof(device_t));
    fs_t *fs_tty = (fs_t*)kmalloc(sizeof(fs_t));
    input_buffer = (uint8_t*)kmalloc(DEFAULT_BUFFER_SIZE);
    output_buffer = (uint8_t*)kmalloc(DEFAULT_BUFFER_SIZE);

    assert(dev_tty != NULL && fs_tty != NULL && input_buffer != NULL && output_buffer != NULL);
    memset(dev_tty, 0, sizeof(device_t));
    memset(fs_tty, 0, sizeof(fs_tty));
    memset(input_buffer, 0, DEFAULT_BUFFER_SIZE);
    memset(output_buffer, 0, DEFAULT_BUFFER_SIZE);

    strncpy(dev_tty->name, "tty", sizeof(dev_tty->name));
    dev_tty->uid = 1;
    dev_tty->type = CHAR;
    dev_tty->filesystem = (void*)fs_tty;
    dev_tty->read = &tty_read_stub;
    dev_tty->write = &tty_write_stub;
    dev_tty->data = 0;

    fs_tty->probe = &tty_probe;
    fs_tty->read = &tty_read_file;
    fs_tty->read_dir = &tty_return_false;
    fs_tty->open = &tty_open;
    fs_tty->open_dir = &tty_return_false;
    fs_tty->write = &tty_write_file;
    fs_tty->exists = &tty_exists;
    fs_tty->mount = &tty_mount;
    fs_tty->unmount = &tty_unmount;

    device_register(dev_tty);
    dev_tty = device_get_by_uid(1);
    bool mounted = vfs_mount("/dev/tty", dev_tty);

    printf("Attempted mount of '/dev/tty': %s\n", (mounted ? "succeess" :
        "failed"));

    // TODO: signal to printf puts somehow to use the file instead
}
