#include <stdint.h>
#include <stdio.h>
#include <drivers/ps2_keyboard.h>
#include <queue.h>
#include <arch/i386/descriptor_tables.h>
#include <kernel/vfs.h>

static queue_t *keyboard_key_queue;
// Bits: 0-7 in order: Alt-Control-Shift
static bool shift = false;
static bool alt = false;
static bool control = false;
int ps2_keyboard_port = 0;

// Corresponds to scancode set 2 which is the only once definitely going to be 
// present on a device
uint8_t charset[] = {
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, '\t', '`', 0,
    0, 0, 0, 0,
    0, 'q', '1', 0,
    0, 0, 'z', 's',
    'a', 'w', '2', 0,
    0, 'c', 'x', 'd',
    'e', '4', '3', 0,
    0, ' ', 'v', 'f',
    't', 'r', '5', 0,
    0, 'n', 'b', 'h',
    'g', 'y', '6', 0,
    0, 0, 'm', 'j',
    'u', '7', '8', 0,
    0, ',', 'k', 'i',
    'o', '0', '9', 0,
    0, '.', '/', 'l',
    ';', 'p', '-', 0,
    0, 0, '\'', 0,
    '[', '=', 0, 0,
    0, 0, '\n', ']',
    0, '\\', 0, 0,
    0, 0, 0, 0,
    0, 0, '\b', 0,
    0, '1', 0, '4',
    '7', 0, 0, 0,
    '0', '-', '2', '5',
    '6', '8', 0, 0,
    0, '+', '3', '-',
    '*', '9', 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,

    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, '\t', '`', 0,
    0, 0, 0, 0,
    0, 'Q', '!', 0,
    0, 0, 'Z', 'S',
    'A', 'W', '@', 0,
    0, 'C', 'X', 'D',
    'E', '$', '#', 0,
    0, ' ', 'V', 'F',
    'T', 'R', '%', 0,
    0, 'N', 'B', 'H',
    'G', 'Y', '^', 0,
    0, 0, 'M', 'J',
    'U', '&', '*', 0,
    0, '<', 'K', 'I',
    'O', ')', '(', 0,
    0, '>', '?', 'L',
    ':', 'P', '_', 0,
    0, 0, '"', 0,
    '{', '+', 0, 0,
    0, 0, '\n', '}',
    0, '?', 0, 0,
    0, 0, 0, 0,
    0, 0, '\b', 0,
    0, '1', 0, '4',
    '7', 0, 0, 0,
    '0', '-', '2', '5',
    '6', '8', 0, 0,
    0, '+', '3', '-',
    '*', '9', 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0
};

void ps2_wait_read() {
    while (1) {
        if (inb(PS2_STATUS_REGISTER) & 1) {
            break;
        }
    }
}

void ps2_wait_write() {
    while (1) {
        if ((inb(PS2_STATUS_REGISTER) & 1) == 0) {
            break;
        }
    }
}

void ps2_read_result() {
    ps2_wait_read();
    return inb(PS2_DATA_PORT);
}

// TODO: Debug
bool ps2_set_scancode_set(uint8_t set) {
    // The top 4 bits have to be zero

    ps2_wait_write();
    outb(PS2_DATA_PORT, PS2_SET_SCANDCODE_SET);
    ps2_wait_write();
    outb(PS2_DATA_PORT, set);
    
    ps2_wait_read();
    return false;
}

bool ps2_self_test() {
    ps2_wait_write();
    outb(PS2_COMMAND_REGISTER, PS2_SELF_TEST);
    ps2_wait_read();

    return inb(PS2_DATA_PORT) == 0x55;
}

uint8_t ps2_read_config() {
    ps2_wait_write();
    outb(PS2_COMMAND_REGISTER, PS2_READ_COMMAND_BYTE);
    ps2_wait_read();
    
    return inb(PS2_DATA_PORT);
}

void ps2_disable(char device) {
    if (device & 1) { 
        ps2_wait_write();
        outb(PS2_COMMAND_REGISTER, PS2_DISABLE_KEYBOARD);
    }

    if (device & 2) {
        ps2_wait_write();
        outb(PS2_COMMAND_REGISTER, PS2_DISABLE_MOUSE);
    }
}

void ps2_enable() {
    uint8_t config = ps2_read_config();

    ps2_wait_write();
    outb(PS2_COMMAND_REGISTER, PS2_ENABLE_KEYBOARD);

    config |= 0b00000001;
    ps2_wait_write();
    outb(PS2_COMMAND_REGISTER, PS2_WRITE_COMMAND_BYTE);
    outb(PS2_DATA_PORT, config);
}

void ps2_flush_output_buffer() {
    char status = inb(PS2_STATUS_REGISTER);

    while (status & 1) {
        inb(PS2_DATA_PORT);
        status = inb(PS2_STATUS_REGISTER);
    }
}

void ps2_reset() {
    ps2_wait_write();
    outb(PS2_COMMAND_REGISTER, 0xFF);
}

void ps2_disable_interrupts() {
    uint8_t command_byte = ps2_read_config();
    command_byte ^= 0b01000011;

    ps2_wait_write();
    outb(PS2_COMMAND_REGISTER, PS2_WRITE_COMMAND_BYTE);
    outb(PS2_DATA_PORT, command_byte);
}

bool ps2_check_port(int port) {
    if (port == 1) {
        ps2_wait_write();
        outb(PS2_COMMAND_REGISTER, PS2_INTERFACE_1_TEST);
        ps2_wait_read();
        if (inb(PS2_DATA_PORT) != 0) return false;

        uint8_t config = ps2_read_config();
        return config & 0b00010000;
    } else if (port == 2) {
        ps2_wait_write();
        outb(PS2_COMMAND_REGISTER, PS2_INTERFACE_2_TEST);
        ps2_wait_read();
        if (inb(PS2_DATA_PORT) != 0) return false;

        uint8_t config = ps2_read_config();
        return config & 0b00100000;
    }
}

int ps2_identify(int port) {
    int type = 0;
    if (port == 2) {
        ps2_wait_write();
        outb(PS2_COMMAND_REGISTER, 0xD4);
    }

    ps2_wait_write();
    outb(PS2_DATA_PORT, PS2_DISABLE_SCANNING);
    ps2_poll_ack();

    if (port == 2) {
        ps2_wait_write();
        outb(PS2_COMMAND_REGISTER, 0xD4);
    }
 
    ps2_wait_write();
    outb(PS2_DATA_PORT, PS2_IDENTIFY);
    ps2_poll_ack();
    ps2_wait_read();
    uint8_t first = inb(PS2_DATA_PORT);
    uint8_t second = inb(PS2_DATA_PORT);

    if (first == 0x00 || first == 0x03 || first == 0x04) {
        type = PS2_MOUSE;
    } else if (first == 0xAB) {
        type = PS2_KEYBOARD;
    } else {
        panic("Unknown device");
    }
    
    if (port == 2) {
        ps2_wait_write();
        outb(PS2_COMMAND_REGISTER, 0xD4);
    }

    ps2_wait_write();
    outb(PS2_DATA_PORT, PS2_ENABLE_SCANNING);

    return type;
}

void ps2_poll_ack() {
    ps2_wait_read();
    assert(inb(PS2_DATA_PORT) == PS2_ACK);
}

bool ps2_keyboard_probe(device_t *dev) {
    return dev->uid == 0;
}

/*
 * The theory here is that the keyboard will read and handle interrupts, then
 * pass the information to the active process's stdin buffer. However, since the
 * system does not support multiple processes, it will just append it to the
 * kernel's stdin buffer for now.
 */
void ps2_keyboard_interrupt_handler(registers_t *regs) {
    ps2_wait_read();
    uint8_t scancode = inb(PS2_DATA_PORT);
    
    if (scancode == PS2_KEYBOARD_LEFT_ALT_PRESSED) {
        alt = true;
    } 
    
    if (scancode == PS2_KEYBOARD_LEFT_CONTROL_PRESSED) {
        control = true;
    } 
    
    if (scancode == PS2_KEYBOARD_LEFT_SHIFT_PRESSED || 
        scancode == PS2_KEYBOARD_RIGHT_SHIFT_PRESSED) {
            shift = true;
    } 
    
    if (scancode == PS2_KEYBOARD_MULTIMEDIA) {
        ps2_wait_read();
        scancode = inb(PS2_DATA_PORT);

        if (scancode == PS2_KEYBOARD_RIGHT_ALT_PRESSED) {
            alt = true;
        } else if (scancode == PS2_KEYBOARD_RIGHT_CONTROL_PRESSED) {
            control = true;
        }
    } 
    
    if (scancode == PS2_KEYBOARD_KEY_RELEASED) {
        ps2_wait_read();
        scancode = inb(PS2_DATA_PORT);

        if (scancode == PS2_KEYBOARD_LEFT_ALT_RELEASED) {
            alt = false;
        } 
        
        if (scancode == PS2_KEYBOARD_LEFT_CONTROL_RELEASED) {
            control = false;
        } 
        
        if (scancode == PS2_KEYBOARD_LEFT_SHIFT_RELEASED || scancode == PS2_KEYBOARD_RIGHT_SHIFT_RELEASED) {
            shift = false;
        } 
        
        if (scancode == PS2_KEYBOARD_MULTIMEDIA) {
            ps2_wait_read();
            scancode = inb(PS2_DATA_PORT);

            if (scancode == PS2_KEYBOARD_RIGHT_ALT_RELEASED) {
                alt = false;
            } else if (scancode == PS2_KEYBOARD_RIGHT_CONTROL_RELEASED) {
                control = false;
            }
        } else{
            // Ignore release keys.
            return;
        }
    }

    if (shift) {
        printf("%c", charset[scancode + 136]);
    } else {
        printf("%c", charset[scancode]);
    }
}

void ps2_keyboard_init() {
    bool has_port_1 = false;
    bool has_port_2 = false;
    int keyboard_port = -1;
    int mouse_port = -1;

    keyboard_key_queue = queue_create();

    printf("Beginning PS/2 Setup...\n");
    ps2_disable(3);
    ps2_flush_output_buffer();
    ps2_disable_interrupts();
    if (!ps2_self_test()) {panic("PS/2 self test failed.");}
    
    if (ps2_check_port(1)) has_port_1 = true;
    if (ps2_check_port(2)) has_port_2 = true;
    
    ps2_enable();
    ps2_reset();
    
    if (has_port_1) {
        if (ps2_identify(1) == PS2_KEYBOARD) {
            printf("PS/2 setup complete\n");
        }
    } else {
        printf("Failed to discover PS/2 keyboard.\n");
    }


    //=====
    // Begin OS-level device initialization
    //=====
    file_t *stdin = vfs_open("/dev/stdio/0");
    /*
    device_t *dev_keyboard = (device_t*)kmalloc(sizeof(device_t));
    fs_t *fs_keyboard = (fs_t*)kmalloc(sizeof(fs_t));

    assert(dev_keyboard != NULL && fs_keyboard != NULL);
    memset(dev_keyboard, 0, sizeof(device_t));
    memset(fs_keyboard, 0, sizeof(fs_t));

    strncpy(dev_keyboard->name, "keyboard", sizeof(dev_keyboard->name));
    dev_keyboard->type = CHAR;
    dev_keyboard->uid = 0;
    dev_keyboard->filesystem = fs_keyboard;
    dev_keyboard->read = &ps2_keyboard_read;

    strncpy(fs_keyboard->name, "keyboard", sizeof(fs_keyboard->name));
    fs_keyboard->read = &ps2_keyboard_read_file;
    fs_keyboard->mount = &ps2_keyboard_mount;
    fs_keyboard->unmount = &ps2_keyboard_unmount;

    dev_keyboard->filesystem = (void*)fs_keyboard;
    device_register(dev_keyboard);
    register_interrupt_handler(33, ps2_keyboard_interrupt_handler); 

    bool mounted = vfs_mount("/dev/keyboard", dev_keyboard);
    printf("Attempted mount of '/dev/keyboard': %s\n", (mounted ? "success" : "failed"));*/
    register_interrupt_handler(33, ps2_keyboard_interrupt_handler); 
}
