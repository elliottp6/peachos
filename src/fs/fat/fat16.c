#include "fat16.h"
#include "status.h"
#include "string/string.h"
#include <stddef.h>
#include <stdint.h>

// fat16 globals
#define PEACHOS_FAT16_SIGNATURE 0x29
#define PEACHOS_FAT16_FAT_ENTRY_SIZE 0x02
#define PEACHOS_FAT16_BAD_SECTOR 0xFF7
#define PEACHOS_FAT16_UNUSED 0x00

// in-memory flag (not on disk)
typedef uint32_t FAT_ITEM_TYPE;
#define FAT_ITEM_TYPE_DIRECTORY 0
#define FAT_ITEM_TYPE_FILE 1

// fat directory entry attributes bitmask
#define FAT_FILE_READ_ONLY 0x01
#define FAT_FILE_HIDDEN 0x02
#define FAT_FILE_SYSTEM 0x04
#define FAT_FILE_VOLUME_LABEL 0x08
#define FAT_FILE_SUBDIRECTORY 0x10
#define FAT_FILE_ARCHIVED 0x20
#define FAT_FILE_DEVICE 0x40
#define FAT_FILE_RESERVED 0x80

// fat16 header (same as in boot.asm)
struct fat_header {
    uint8_t short_jump_ins[3];
    uint8_t oem_identifier;
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t fat_copies;
    uint16_t root_dir_entries;
    uint16_t number_of_sectors;
    uint8_t media_type;
    uint16_t sectors_per_fat;
    uint16_t sectors_per_track;
    uint16_t number_of_heads;
    uint32_t hidden_sectors;
    uint32_t sectors_bigs;
} __attribute((packed));

// fat16 extended header (same as in boot.asm)
struct fat_header_extended {
    uint8_t drive_number;
    uint8_t win_nt_bit;
    uint8_t signature;
    uint32_t volume_id;
    uint8_t volume_id_string[11];
    uint8_t system_id_string[8];
} __attribute__((packed));

// full fat16 header (same as in boot.asm)
struct fat_h {
    struct fat_header primary_header;
    union fat_h_e {
        struct fat_header_extended extended_header;
    } shared;
};

// forward declarations
int fat16_resolve( struct disk* disk );
void* fat16_open( struct disk* disk, struct path_part* path, FILE_MODE mode );

// FAT16 filesystem structure
struct filesystem fat16_fs = {
    .resolve = fat16_resolve,
    .open = fat16_open
};

// functions
struct filesystem* fat16_init() {
    strcpy( fat16_fs.name, "FAT16" );
    return &fat16_fs;
}

int fat16_resolve( struct disk* disk ) {
    // TODO: at some point, we should examine the disk to see if it's actually FAT16
    // but for now, just assume that it is
    return 0;
}

void* fat16_open( struct disk* disk, struct path_part* path, FILE_MODE mode ) {
    // TODO: implement me!
    return NULL;
}
