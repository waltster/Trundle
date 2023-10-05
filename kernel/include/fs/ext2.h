#ifndef _EXT2_H
#define _EXT2_H 1

#include <stdint.h>

#define EXT2_CONVERT_SIZE(b) (1024 << b)
#define EXT2_SIGNATURE 0xEF53
#define EXT2_STATE_CLEAN 1
#define EXT2_STATE_ERROR 2
#define EXT2_ERROR_IGNORE 1
#define EXT2_ERROR_REMOUNT_RDONLY 2
#define EXT2_ERROR_PANIC 3
#define EXT2_CREATOR_LINUX 0
#define EXT2_CREATOR_OTHER 4
#define EXT2_FEATURE_PREALLOCATE_DIR 0x1
#define EXT2_FEATURE_AFS_SERVER_INODES 0x2
#define EXT2_FEATURE_HAS_JOURNAL 0x4
#define EXT2_FEATURE_INODES_EXTENDED 0x8
#define EXT2_FEATURE_RESIZEABLE_FS 0x10
#define EXT2_FEATURE_DIR_HASH_INDEX 0x20
#define EXT2_COMPRESSION_USED 0x1
#define EXT2_DIRECTORIES_CONTAIN_TYPE 0x2
#define EXT2_FS_NEEDS_REPLAY_JOURNAL 0x4
#define EXT2_FS_USES_JOURNAL_DEVICE 0x8
#define EXT2_SPARSE_SUPERBLOCKS 0x1
#define EXT2_64_BIT_FILE_SIZE 0x2
#define EXT2_DIRECTORY_IN_BINARY_TREE 0x4
#define EXT2_TYPE_FIFO 0x1000
#define EXT2_TYPE_CHAT 0x2000
#define EXT2_TYPE_DIR  0x4000
#define EXT2_TYPE_BLOCK 0x6000
#define EXT2_TYPE_FILE 0x8000
#define EXT2_SYMBOLIC_LINK 0xA000
#define EXT2_UNIX_SOCKER 0xC000

#define EXT2_OTHER_EXEC 0x001
#define EXT2_OTHER_WRITE 0x002
#define EXT2_OTHER_READ 0x004
#define EXT2_GROUP_EXEC 0x008
#define EXT2_GROUP_WRITE 0x010
#define EXT2_GROUP_READ 0x020
#define EXT2_USER_EXEC 0x040
#define EXT2_USER_WRITE 0x080
#define EXT2_USER_READ 0x100
#define EXT2_STICKY_BIT 0x200
#define EXT2_SET_GID 0x400
#define EXT2_SET_UID 0x800

#define EXT2_FLAG_SECURE_DELETION 0x1
#define EXT2_FLAG_KEEP_COPY 0x2
#define EXT2_FLAG_FILE_COMPRESSION 0x4
#define EXT2_FLAG_SYNCHRONOUS_UPDATE 0x8
#define EXT2_FLAG_IMMUTABLE_FILE 0x10
#define EXT2_FLAG_APPEND_ONLY 0x20
#define EXT2_FLAG_NO_DUMP 0x40
#define EXT2_FLAG_NO_UPDATE_ACCESS 0x80
#define EXT2_FLAG_HASH_INDEX 0x10000
#define EXT2_FLAG_AFS_DIR 0x20000
#define EXT2_FLAG_JOURNAL_FILE_DATA 0x4000

#define EXT2_DIRENT_TYPE_UNKNOWN 0
#define EXT2_DIRENT_TYPE_FILE 1
#define EXT2_DIRENT_TYPE_DIR 2
#define EXT2_DIRENT_TYPE_CHAR 3
#define EXT2_DIRENT_TYPE_BLOCK 4
#define EXT2_DIRENT_TYPE_FIFO 5
#define EXT2_DIRENT_TYPE_SOCKET 6
#define EXT2_DIRENT_TYPE_SYMBOLIC_LINK 7

typedef struct {
    uint32_t total_inodes;
    uint32_t total_blocks;
    uint32_t total_blocks_reserved;
    uint32_t total_blocks_unallocated;
    uint32_t total_inodes_unallocated;
    uint32_t superblock_block_number;
    uint32_t block_size;
    uint32_t fragment_size;
    uint32_t number_blocks_to_block_group;
    uint32_t number_fragments_to_block_group;
    uint32_t number_inodes_to_block_group;
    uint32_t last_mount_time;
    uint32_t last_written_time;
    uint16_t times_mounted_since_check;
    uint16_t times_allowed_mounted_before_check;
    uint16_t signature;
    uint16_t state;
    uint16_t error_handling;
    uint16_t version_minor;
    uint32_t time_last_check;
    uint32_t interval_forced_check;
    uint32_t operating_system_id;
    uint32_t version_major;
    uint16_t uid_for_reserved;
    uint16_t gid_for_reserved;
} __attribute__((packed)) ext2_superblock_t;

typedef struct {
    ext2_superblock_t superblock;
    uint32_t first_non_reserved;
    uint16_t inode_size;
    uint16_t superblock_block_group;
    uint32_t optional_features_present;
    uint32_t required_features_present;
    uint32_t features_or_read_only;
    uint8_t filesystem_id[16];
    uint8_t volume_name[16];
    uint8_t path_to_last_mount[64];
    uint32_t compression_used;
    uint8_t numb_blocks_prealloc_files;
    uint8_t numb_blocks_prealloc_directories;
    uint16_t unused;
    uint8_t journal_id[16];
    uint32_t journal_inode;
    uint32_t journal_device;
    uint32_t head_of_orphan_inode_list;
    uint8_t unused_1[788];
} __attribute__((packed)) ext2_superblock_extended_t;

typedef struct {
    uint32_t addr_block_usage_bitmap;
    uint32_t addr_inode_usage_bitmap;
    uint32_t addr_starting_inode_table;
    uint16_t num_unalloc_blocks_in_group;
    uint16_t num_unalloc_inodes_in_group;
    uint16_t num_dirs_in_group;
    uint8_t unused[14];
} __attribute__((packed)) ext2_block_group_descriptor_t;

typedef struct {
    uint16_t type_permissions;
    uint16_t uid;
    uint32_t size_low;
    uint32_t last_access_time;
    uint32_t creation_time;
    uint32_t last_modification_time;
    uint32_t deletion_time;
    uint16_t gid;
    uint16_t num_hard_links;
    uint32_t num_disk_sectors;
    uint32_t flags;
    uint32_t os_specific;
    uint32_t direct_block_ptrs[12];
    uint32_t signly_indirect_block_ptr;
    uint32_t double_indirect_block_ptr;
    uint32_t triply_indirect_block_ptr;
    uint32_t generation_number;
    uint32_t extended_attribute_block;
    uint32_t reserved;
    uint32_t addr_block_fragment;
    uint8_t  os_specific_1[12];
} __attribute__((packed)) ext2_inode_t;

typedef struct {
    uint32_t inode;
    uint16_t size;
    uint8_t name_length;
    uint8_t type;
    // Name is here, but variable length
} __attribute__((packed)) ext2_dirent;

#endif
