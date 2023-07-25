#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>

#include "../kernel/include/drivers/myfs.h"

int main(int argc, char** argv) {
    if (argc < 3) {
        printf("usage: %s <output file> <input file 1> [...]\n", argv[0]);
        exit(1);
    }

    char *output_file = argv[1];
    int fd_outfile;
    myfs_header_t header;
    myfs_file_header_t tmp_header;
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    header.magic = MYFS_MAGIC;
    snprintf((char*)&header.date_made, 11, "%d-%02d-%02d", tm.tm_year + 1900, 
            tm.tm_mon + 1, tm.tm_mday);
    snprintf((char*)&header.time_made, 9, "%02d:%02d:%02d", tm.tm_hour, tm.tm_min, 
            tm.tm_sec);
    
    if ((fd_outfile = open(output_file, O_RDWR | O_CREAT, 0660)) < 0) {
        perror("Failed to open output file for writing.");
        exit(1);
    }

    write(fd_outfile, &header, sizeof(myfs_header_t));

    for (int i = 2; i < argc; i++) {
        int fd;
        struct stat file_stat;

        memset((void*)&tmp_header, 0, sizeof(myfs_file_header_t));

        if (!(fd = open(argv[i], O_RDONLY))) {
            printf("Warning: Failed to open file. Skipping.\n");
            perror("Failed to open file");
            continue;
        }

        if (fstat(fd, &file_stat) < 0) {
            printf("Warning: Failed to stat file. Skipping.\n");
            perror("Failed to stat file");
            close(fd);
            continue;
        }

        char *buf = (char*)malloc(file_stat.st_size);
        memset(buf, 0, file_stat.st_size);

        printf("Beginning file: %s\n", argv[i]);
        printf("\tFile Size: %d\n", file_stat.st_size);

        if (read(fd, buf, file_stat.st_size) < file_stat.st_size) {
            printf("Error reading file. Skipping.");
            perror("Unable to read entire file");
            free(buf);
            close(fd);
            continue;
        }

        tmp_header.magic = MYFS_MAGIC;
        strncpy(tmp_header.file_name, argv[i], 255);
        tmp_header.file_name[255] = 0;
        tmp_header.file_size = file_stat.st_size;

        if (write(fd_outfile, (void*)&tmp_header, sizeof(myfs_file_header_t))
                < sizeof(myfs_file_header_t)) {
            perror("Failed to write to file. Exiting");
            exit(1);
        }

        if (write(fd_outfile, (void*)buf, tmp_header.file_size) < 
                tmp_header.file_size) {
            perror("Failed to write to file. Exiting");
            exit(1);
        }

        free(buf);
        close(fd);

        printf("Successfully wrote file: %s\n", argv[i]);
    }

    close(fd_outfile);
}
