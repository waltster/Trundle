#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <kernel/vmm.h>
#include <kernel/vfs.h>

static device_t *devices;
static int num_devices = 0;

// Register a new device with the manager, receive a UID unique to this boot
int device_register(device_t *d) {
    if (d == NULL) return -1;

    devices[num_devices] = *d;
    num_devices++;

    return num_devices - 1;
}

device_t *device_get_by_uid(int uid) {
    for (int i = 0; i < INITIAL_DEVICE_COUNT; i++) {
        if (devices[i].uid == uid) {
            return &devices[i];
        }
    }

    return -1;
}

void device_printall() {
    if (num_devices <= 0) {
        printf("No devices.\n");
        return;
    }

    for (int i = 0; i < num_devices; i++) {
        device_t dev = devices[i];
        printf("\tdevice #%d (%d, '%s', %s)\n", i, dev.uid, dev.name, 
                (dev.type == 0 ? "char" : "block"));
    }
}

void device_init() {
    devices = (device_t*)kmalloc(sizeof(device_t)*INITIAL_DEVICE_COUNT);
    assert(devices != NULL);
    memset(devices, 0, sizeof(device_t)*INITIAL_DEVICE_COUNT);

    printf("Device manager initialized.\n");
}
