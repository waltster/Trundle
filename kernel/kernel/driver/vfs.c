#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <drivers/initrd.h>
#include <kernel/vfs.h>
#include <kernel/vmm.h>

static mount_point_t **mount_points;
static int num_mount_points = 0;

inline bool remove_until(char *str, char c) {
    int i = strlen(str);

    while (i) {
        i--;

        if (str[i] == c) {
            str[i + 1] = '\0';
            return true;
        }

    }

    return false;
}

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

file_t *vfs_open(char *location) {
    if (location == NULL) {
        printf("Location is null!\n");    
        return NULL; 
    }

    int beginning_of_file = 0;
    int mt_index = vfs_get_mount_point_index(location, &beginning_of_file);

    if (mt_index < 0) {
        printf("Location not mounted!\n");     
        return NULL;
    }

    mount_point_t *mountpoint = mount_points[mt_index];

    return ((fs_t*)mountpoint->device->filesystem)->open(
            location, mountpoint->device);
}

int vfs_read(file_t *fp, char *buffer, int length) {
    if (!fp || !buffer) {
        printf("File buffer or file missing!\n");
        return -1;
    }

    if (length <= 0) return 0;

    mount_point_t *mountpoint = mount_points[fp->mount_index];

    if (!mountpoint) {
        printf("Warning: attempted to read from unmounted device\n");
        return -1;
    }

    device_t *dev = (device_t*)mountpoint->device;
    fs_t *filesystem = (fs_t*)dev->filesystem;

    return filesystem->read(fp, buffer, length, dev);
}

/*
 * Find what part of a given path is the mount point and what index of the
 * string is that mountpoint found at.
 *
 * @return the index in mountpoints
 */
int vfs_get_mount_point_index(char *location, int *str_index) {
    char *filename = (char*)kmalloc(strlen(location) + 1);
    memset(filename, 0, strlen(location) + 1);
    memcpy(filename, location, strlen(location) + 1);
    
    if (filename[strlen(filename)] == '/') remove_until(filename, '/');

    while (true) {
        for (int i = 0; i < num_mount_points; i++) {
            if (strcmp(mount_points[i]->location, filename) == 0) {
               *str_index = strlen(filename) - 1;
               kfree(filename);
               return i; 
            }
        } 

        if (strcmp(filename, "/") == 0) break;

        remove_until(filename, '/');
    }

    kfree(filename);
    return -1;
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
