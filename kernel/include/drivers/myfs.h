#ifndef _MYFS_H
#define _MYFS_H 1

#include <stdint.h>

#define MYFS_MAGIC 0xB33FD00D

typedef struct {
    uint32_t magic;
    char date_made[12];
    char time_made[10];
} myfs_header_t;

typedef struct {
    uint32_t magic;
    char file_name[256];
    uint32_t file_size;
} myfs_file_header_t;

#endif
