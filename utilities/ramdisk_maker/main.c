#include <stdio.h>
#include <stdlib.h>

typedef struct {
    char file_name[256];
    int file_size;
    void *file_content;
} ramdisk_file_t;

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("usage: %s <file1> [file2...]\n", argv[0]);
        exit(1);
    }

    printf("Ramdisk Maker v1\n");
    
    for (int i = 1; i < argc; i++) {
        ramdisk_file_t* file = (ramdisk_file_t*)calloc(1, 
                sizeof(ramdisk_file_t));

        printf("File: %s\n", argv[i]);
        strncpy(file->file_name, argv[i], 255);
        file->file_name[255] = 0;

        if (access(argv[i], O_RDONLY) == -1) {
            perror("Cannot access requested file");
            exit(2);
        }

        int fd = open(argv[i], O_RDONLY);
    }
}
