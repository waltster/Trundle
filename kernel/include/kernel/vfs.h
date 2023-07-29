#ifndef _VFS_H
#define _VFS_H 1

#include <stdint.h>

#define MAX_LEN_FILESYSTEM_NAME 128
#define MAX_LEN_DEVICE_NAME 128
#define MAX_MOUNT_POINTS 16
#define INITIAL_DEVICE_COUNT 16

typedef enum {
    CHAR = 0,
    BLOCK = 1
} device_type_t;

typedef struct device {
    char name[MAX_LEN_DEVICE_NAME];
    uint32_t uid;
    device_type_t type;
    void *filesystem;
    uint32_t (*read)(uint8_t *, uint32_t, uint32_t, struct device *);
    uint32_t (*write)(uint8_t *, uint32_t, uint32_t, struct device *);
    void *data;
} device_t;

typedef struct {
    uint8_t is_dir;
    uint16_t flags;
    int mount_index;  // For quick caching
    int read_pos;     // For reading
    int file_pos;     // For readdir if directory
    uint32_t data;
    uint32_t file_size;
} file_t;

typedef struct {
    char name[MAX_LEN_FILESYSTEM_NAME];
    uint32_t (*probe)(device_t*);
    file_t* (*open)(char *location, device_t *);
    uint32_t (*read)(file_t*, char*, uint32_t, device_t *);
    file_t* (*read_dir)(file_t *dir, device_t *);
    file_t* (*open_dir)(char *, device_t *);
    uint32_t (*write)(file_t *, char *, uint32_t, device_t *);
    uint32_t (*exists)(char *, device_t *);
    uint32_t (*mount)(char *, device_t *);
    uint32_t (*unmount)(device_t *);
} fs_t;

typedef struct {
    char *location;
    device_t *device;
} mount_point_t;

bool vfs_is_mounted(char *mount);
device_t *vfs_get_mount(char *location);
file_t *vfs_open(char *location);
int vfs_read(file_t *fp, char *buffer, int length);
int vfs_close(file_t *fp);
bool vfs_mount(char *mount, device_t *dev);
int vfs_get_mount_point_index(char *location, int *str_index);

int device_register(device_t *d);
device_t *device_get_by_uid(int uid);
void device_printall();
void device_init();

#endif
