#include <stdint.h>
#include <stdbool.h>
#include <kernel/vfs.h>
#include <kernel/vmm.h>

static mount_point_t **mount_points;
static int num_mount_points = 0;

bool vfs_is_mounted(char *mount) {
    for (int i = 0; i < num_mount_points; i++) {
        if (strcmp(mount_points[i]->location, mount) == 0) return true;
    }

    return false;
}

device_t *vfs_get_mount(char *location) {
    for (int i = 0; i < num_mount_points; i++) {
        if (strcmp(mount_points[i]->location, location) == 0) {
            return mount_points[i];
        }
    }

    return NULL;
}

bool vfs_mount(char *mount, device_t *dev) {
    if (!dev || !(dev->uid)) return false;
    if (vfs_is_mounted(mount)) return false;

    if (initrd_probe(dev)) {
        if (initrd_mount(mount, dev)) {
            mount_point_t *new_point = (mount_point_t*)kmalloc(
                    sizeof(mount_point_t));
            memset(new_point, 0, sizeof(mount_point_t));

            new_point->location = mount;
            new_point->device = dev;
            mount_points[num_mount_points++] = new_point;

            return true;
        }

        return false;
    }

    return false;
}
