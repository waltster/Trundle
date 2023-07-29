#ifndef _VFS_H
#define _VFS_H 1

#include <stdint.h>

#define MAX_LEN_FILESYSTEM_NAME 128
#define MAX_LEN_DEVICE_NAME 128
#define INITIAL_DEVICE_COUNT 16

typedef enum {
    CHAR = 0,
    BLOCK = 1
} device_type_t;

typedef struct {
    char name[MAX_LEN_DEVICE_NAME];
    uint32_t id;
    device_type_t;
    void *filesystem;
    uint32_t (*read)(uint8_t *, uint32_t, uint32_t, struct device_t *);
    uint32_t (*write)(uint8_t *, uint32_t, uint32_t, struct device_t *);
} device_t;

typedef struct {
    char name[MAX_LEN_FILESYSTEM_NAME];
    uint32_t (*probe)(device_t*);
    uint32_t (*read)(char *, char*, uint32_t, device_t *);
    uint32_t (*read_dir)(char *, char *, device_t *);
    uint32_t (*write)(char *, char *, uint32_t, device_t *);
    uint32_t (*exists)(char *, device_t *);
    uint32_t (*mount)(char *, device_t *);
    uint32_t (*unmount)(device_t *);
} fs_t;

#endif
