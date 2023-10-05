#include <stdio.h>
#include "kernel/include/fs/ext2.h"

int main() {
    printf("%d\n%d\n%d", sizeof(ext2_superblock_t), sizeof(ext2_superblock_extended_t), sizeof(ext2_superblock_t) + sizeof(ext2_superblock_extended_t));
}
