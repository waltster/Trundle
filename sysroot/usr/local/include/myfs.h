#ifndef _MYFS_H
#define _MYFS_H 1

#include <stdint.h>

#define MYFS_MAGIC 0xB33FD00D

typedef struct {
    uint32_t magic;
    char date_made[11];  // YYYY-MM-DD\0 format
    char time_made[9];   // 00:00:00\0 format
    uint32_t reserved;
} __attribute__((packed)) myfs_header_t;

typedef struct {
    uint32_t magic;
    char file_name[256];
    uint32_t file_size;
} __attribute__((packed)) myfs_file_header_t;

#endif
