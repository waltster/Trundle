#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include "../kernel/include/drivers/myfs.h"

int main(int argc, char **argv) {
    if (argc < 3) {
        printf("usage: %s <outfile> <infile> [...]\n", argv[0]);
        exit(1);
    }

    uint8_t *buffer = (uint8_t*)malloc(sizeof(myfs_header_t));
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    
    myfs_header_t *header = (myfs_header_t*)buffer;

    header->magic = MYFS_MAGIC;
    snprintf((char*)&header->date_made, 11, "%d-%02d-%02d", tm.tm_year + 1900,
            tm.tm_mon + 1, tm.tm_mday);
    snprintf((char*)&header->time_made, 9, "%02d:%02d:%02d", tm.tm_hour, tm.tm_min,
            tm.tm_sec);

    int pos = sizeof(myfs_header_t);
    int num_files = 0;

    for (int i = 2; i < argc; i++) {
        int fd;
        struct stat file_stat;
    
        if (!(fd = open(argv[i], O_RDONLY))) {
            perror("Failed to open file. Skipping.");
            continue;
        }

        if (fstat(fd, &file_stat) < 0) {
            perror("Failed to stat file. Skipping");
            close(fd);
            continue;
        }

        buffer = realloc(buffer, pos + sizeof(myfs_file_header_t) + 
                (int)file_stat.st_size);

        myfs_file_header_t *f_header = (myfs_file_header_t*)(buffer + pos);
        strncpy(f_header->file_name, argv[i], 255);
        f_header->file_name[255] = 0;
        f_header->file_size = file_stat.st_size;

        pos += sizeof(myfs_file_header_t);

        if (read(fd, buffer + pos, file_stat.st_size) < file_stat.st_size) {
            perror("Failed reading file.");
            buffer = realloc(buffer, pos); 
            close(fd);
            continue;
        }

        num_files++;
        pos += file_stat.st_size;
        close(fd);

        printf("Successfully wrote file %s\n", argv[i]);
    }

    header = (myfs_header_t*)buffer;
    header->total_length = pos;
    
    int outfile_fd = open(argv[1], O_RDWR | O_CREAT, 0660);

    if (write(outfile_fd, buffer, pos) < pos) {
        perror("Failed to write all the bytes");
        close(outfile_fd);
    }

    free(buffer);
}
