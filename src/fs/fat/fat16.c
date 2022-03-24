#include "fat16.h"
#include "status.h"
#include "string/string.h"
#include "disk/disk.h"
#include "disk/streamer.h"
#include "memory/memory.h"
#include "memory/heap/kheap.h"
#include "kernel.h"
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

// fat directory entry attributes flags
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
    uint8_t oem_identifier[8];
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
    uint32_t sectors_big;
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

struct fat_directory_item {
    uint8_t filename[8];
    uint8_t ext[3];
    uint8_t attribute; // attribute bit flags
    uint8_t reserved;
    uint8_t creation_time_tenths_of_a_sec;
    uint16_t creation_time;
    uint16_t creation_date;
    uint16_t last_access;
    uint16_t high_16_bits_first_cluster;
    uint16_t last_mod_time;
    uint16_t last_mod_date;
    uint16_t low_16_bits_first_cluster;
    uint32_t filesize;
} __attribute__((packed));

struct fat_directory {
    struct fat_directory_item* item;
    int total;
    int sector_pos;
    int ending_sector_pos;
};

struct fat_item {
    union {
        struct fat_directory_item* item;
        struct fat_directory* directory;
    };

    FAT_ITEM_TYPE type;
};

// represents an open file
struct fat_item_descriptor {
    struct fat_item* item;
    uint32_t pos; // position we've seeked to in the actual file
};

// represents the entire fat16 system
struct fat_private {
    struct fat_h header;
    struct fat_directory root_directory;
    struct disk_stream* cluster_read_stream; // used to stream data clusters
    struct disk_stream* fat_read_stream; // used to strema the file allocation table
    struct disk_stream* directory_stream; // used in situations where we stream the directory
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

// initialize private, except for header & root_directory
static void fat16_init_private( struct disk* disk, struct fat_private* private ) {
    memset( private, 0, sizeof( struct fat_private ) );
    private->cluster_read_stream = diskstreamer_new( disk->id );
    private->fat_read_stream = diskstreamer_new( disk->id );
    private->directory_stream = diskstreamer_new( disk->id );
}

int fat16_sector_to_absolute( struct disk* disk, int sector ) { return sector * disk->sector_size; }

int fat16_get_total_items_for_directory( struct disk* disk, uint32_t directory_start_sector ) {
    // allocate directory items
    struct fat_directory_item item;

    // TODO: these lines of code have no impact (why were they in the lecture?)
    //struct fat_directory_item empty_item;
    //memset( &empty_item, 0, sizeof( empty_item ) );

    int res = 0;
    int i = 0;
    int directory_start_pos = directory_start_sector * disk->sector_size;

    // get FAT structure
    struct fat_private* fat_private = disk->fs_private;
    if( NULL == fat_private ) {
        print( "FAILED on fat16_get_total_items: fat_private is NULL\n" );
        res = -EIO;
        goto out;
    }

    // get directory stream
    struct disk_stream* stream = fat_private->directory_stream;
    if( NULL == stream->disk ) {
        print( "FAILED on fat16_get_total_items: disk stream is NULL\n" );
        res = -EIO;
        goto out;
    }
    
    // seek to directory start sector
    if( 0 != diskstreamer_seek( stream, directory_start_pos ) ) {
        print( "FAILED to seek within fat16_get_total_items_for_directory\n" );
        res = -EIO;
        goto out;
    }
    
    // read directory
    while( 1 ) {
        if( 0 != diskstreamer_read( stream, &item, sizeof( item ) ) ) {
            print( "FAILED to read within fat16_get_total_items_for_directory\n" );
            res = -EIO;
            goto out;
        }

        print( "read directory item\n" );

        if( 0x00 == item.filename[0] ) break; // 0x00 indicates last entry
        if( 0xE5 == item.filename[0] ) continue; // 0xE5 indicates that the entry is available
        i++;
    }

    // return the # of items we read
    res = i;

out:
    return res;
}

int fat16_get_root_directory( struct disk* disk, struct fat_private* fat_private, struct fat_directory* directory ) {
    int res = 0;

    // get location of root directory
    struct fat_header* primary_header = &fat_private->header.primary_header;
    int root_dir_sector_pos = primary_header->reserved_sectors + primary_header->fat_copies * primary_header->sectors_per_fat;
    
    // get size of root directory
    int root_dir_entries = fat_private->header.primary_header.root_dir_entries;
    int root_dir_size = root_dir_entries * sizeof( struct fat_directory_item );
    
    // determine # of sectors to read (rounded up)
    int total_sectors = root_dir_size / disk->sector_size;
    if( root_dir_size % disk->sector_size ) total_sectors++;

    // get total # of items in this directory
    int total_items = fat16_get_total_items_for_directory( disk, root_dir_sector_pos );
    if( total_items < 0 ) {
        print( "FAILED to get fat16_get_total_item_for_directory\n" );
        res = -EIO;
        goto out;
    }

    // allocate a fat_directory_item array to hold the root directory items
    struct fat_directory_item* dir = kzalloc( root_dir_size );
    if( !dir ) { res = -ENOMEM; goto out; }

    // seek to the root directory
    struct disk_stream* stream = fat_private->directory_stream;
    if( 0 != diskstreamer_seek( stream, fat16_sector_to_absolute( disk, root_dir_sector_pos ) ) ) {
        res = -EIO;
        goto out;
    }

    // read the root directory
    if( 0 != diskstreamer_read( stream, dir, root_dir_size ) ) { res = -EIO; goto out; }

    // done! fill in the 'directory', which is our result
    directory->item = dir;
    directory->total = total_items;
    directory->sector_pos = root_dir_sector_pos;
    directory->ending_sector_pos = root_dir_sector_pos + root_dir_size / disk->sector_size;

out:
    return res;
}

int fat16_resolve( struct disk* disk ) {
    int res = 0;

    // allocate the filesystem structure
    struct fat_private* fat_private = kzalloc( sizeof( struct fat_private ) );
    fat16_init_private( disk, fat_private );

    // bind filesystem to disk
    disk->fs_private = fat_private;
    disk->filesystem = &fat16_fs;

    // create disk stream
    struct disk_stream* stream = diskstreamer_new( disk->id );
    if( !stream ) { res = -ENOMEM; goto out; }

    // read fat header from 0th position in the stream
    if( 0 != diskstreamer_read( stream, &fat_private->header, sizeof( fat_private->header ) ) ) {
        res = -EIO;
        goto out;
    }

    // check for the FAT16 signature
    if( PEACHOS_FAT16_SIGNATURE != fat_private->header.shared.extended_header.signature ) {
        res = -EFSNOTUS;
        goto out;
    }

    // load the root directory
    // TODO: why do we need to pass fat_private when it's directly inside of disk structure already?
    // seems like a bit messy to me
    if( 0 != fat16_get_root_directory( disk, fat_private, &fat_private->root_directory ) ) {
        print( "fat16 FAILED to get root directory\n" );
        res = -EIO;
        goto out;
    }

out:
    if( stream ) diskstreamer_close( stream );
    if( res < 0 ) {
        kfree( fat_private );
        disk->fs_private = NULL;
    }
    return res;
}

void* fat16_open( struct disk* disk, struct path_part* path, FILE_MODE mode ) {
    // TODO: implement me!
    return NULL;
}
