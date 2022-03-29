#include "fat16.h"
#include "status.h"
#include "string/string.h"
#include "disk/disk.h"
#include "disk/streamer.h"
#include "memory/memory.h"
#include "memory/heap/kheap.h"
#include "string/string.h"
#include "kernel.h"
#include "config.h"
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
struct fat_file_descriptor {
    struct fat_item* item;
    uint32_t pos; // position we've seeked to in the actual file
};

// represents the fat16 system, except for disk info
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
int fat16_read( struct disk* disk, void* descriptor, uint32_t size, uint32_t nmemb, char* out );

// FAT16 VFS structure
struct filesystem fat16_fs = {
    .resolve = fat16_resolve,
    .open = fat16_open,
    .read = fat16_read
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
    // get FAT structure
    struct fat_private* fat_private = disk->fs_private;
    if( NULL == fat_private ) {
        print( "FAILED on fat16_get_total_items: fat_private is NULL\n" );
        return -EIO;
    }

    // get directory stream
    struct disk_stream* stream = fat_private->directory_stream;
    if( NULL == stream->disk ) {
        print( "FAILED on fat16_get_total_items: disk stream is NULL\n" );
        return -EIO;
    }
    
    // seek to directory start sector
    int directory_start_pos = directory_start_sector * disk->sector_size;
    if( 0 != diskstreamer_seek( stream, directory_start_pos ) ) {
        print( "FAILED to seek within fat16_get_total_items_for_directory\n" );
        return -EIO;
    }
    
    // read directory
    struct fat_directory_item item;
    int i = 0;
    while( 1 ) {
        if( 0 != diskstreamer_read( stream, &item, sizeof( item ) ) ) {
            print( "FAILED to read within fat16_get_total_items_for_directory\n" );
            return -EIO;
        }

        // print( "read directory item\n" );
        if( 0x00 == item.filename[0] ) break; // 0x00 indicates last entry
        if( 0xE5 == item.filename[0] ) continue; // 0xE5 indicates that the entry is empty (available)
        i++;
    }
    return i;
}

int fat16_get_root_directory( struct disk* disk, struct fat_directory* directory ) {
    // get location of root directory
    struct fat_private* fat_private = disk->fs_private;
    struct fat_header* primary_header = &fat_private->header.primary_header;
    int root_dir_sector_pos = primary_header->reserved_sectors + primary_header->fat_copies * primary_header->sectors_per_fat;
    
    // get size of root directory
    int root_dir_entries = fat_private->header.primary_header.root_dir_entries,
        root_dir_size = root_dir_entries * sizeof( struct fat_directory_item );
    
    // determine # of sectors to read (rounded up)
    int total_sectors = root_dir_size / disk->sector_size;
    if( root_dir_size % disk->sector_size ) total_sectors++;

    // get total # of items in this directory
    int total_items = fat16_get_total_items_for_directory( disk, root_dir_sector_pos );
    if( total_items < 0 ) {
        print( "FAILED to get fat16_get_total_item_for_directory\n" );
        return -EIO;
    }

    // allocate a fat_directory_item array to hold the root directory items
    struct fat_directory_item* dir = kzalloc( root_dir_size );
    if( !dir ) return -ENOMEM;

    // seek to the root directory
    int res = 0;
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
    if( res < 0 && dir ) kfree( dir );
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
    if( 0 != fat16_get_root_directory( disk, &fat_private->root_directory ) ) {
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

// -- LECTURE 52: implementing FAT16 fopen --

void fat16_get_full_relative_filename( struct fat_directory_item* item, char* out, int max_len ) {
    // clear output string
    memset( out, 0, max_len );
    
    // write filename to out (replacing spaces with null terminator)
    string_replace_terminator_with_null_terminator( &out, (const char*)item->filename, 0x20 );
    
    // append extension (if extension is not empty)
    if( item->ext[0] && item->ext[0] != 0x20 ) {
        *out++ = '.';
        string_replace_terminator_with_null_terminator( &out, (const char*)item->ext, 0x20 );
    }
}

static uint32_t fat16_get_first_cluster( struct fat_directory_item* item ) {
    // note: bugfix from lecture 52: we need to cast to uint32_t and shift the high bits up by 16 beforing or-ing them together
    return ((uint32_t)item->high_16_bits_first_cluster << 16) | (uint32_t)item->low_16_bits_first_cluster;
}

static int fat16_cluster_to_sector( struct fat_private* private, int cluster ) {
    return private->root_directory.ending_sector_pos +
           (cluster - 2) * private->header.primary_header.sectors_per_cluster; // why cluster - 2? RTFM.
}

static int fat16_size_of_cluster_bytes( struct disk* disk ) {
    struct fat_private* private = disk->fs_private;
    return private->header.primary_header.sectors_per_cluster * disk->sector_size;
}

static uint32_t fat16_get_first_fat_sector( struct fat_private* private ) {
    return private->header.primary_header.reserved_sectors;
}

// note that fat_entry is only 16 bits, but we return a int as so we can have negative values for error codes
static int fat16_get_fat_entry( struct disk* disk, int cluster ) {
    // get read stream
    int res = -1;
    struct fat_private* private = disk->fs_private;
    struct disk_stream* stream = private->fat_read_stream;
    if( !stream ) goto out;

    // get table position
    uint32_t fat_table_position = fat16_get_first_fat_sector( private ) * disk->sector_size;
    
    // seek to table position
    if( (res = diskstreamer_seek( stream, fat_table_position * cluster * PEACHOS_FAT16_FAT_ENTRY_SIZE )) < 0 ) goto out;

    // read table entry
    uint16_t result;
    if( (res = diskstreamer_read( stream, &result, sizeof( result ) )) < 0 ) goto out;
    res = result;

out:
    return res;
}

// get the correct cluster to use based on the start_cluster and the offset
static int fat16_get_cluster_for_offset( struct disk* disk, int cluster, int offset ) {    
    int cluster_count = offset / fat16_size_of_cluster_bytes( disk );

    // walk clusters until we hit 'cluster_count' (then we'll have the cluster that contains 'offset')
    for( int i = 0; i < cluster_count; i++ ) {
        // get entry, which should point to next cluster
        cluster = fat16_get_fat_entry( disk, cluster );

        // check for invalid entry
        if( 0xFF8 == cluster || 0xFFF == cluster || // last entry in the file
            PEACHOS_FAT16_BAD_SECTOR == cluster || // bad sector
            0xFF0 == cluster || 0xFF6 == cluster || // reserved sector
            0 == cluster ) // null entry
            return -EIO;
    }
    return cluster;
}

static int fat16_read_internal_from_stream( struct disk* disk, struct disk_stream* stream, int start_cluster, int offset, int total, void* out ) {
    // get cluster
    int res = 0,
        size_of_cluster_bytes = fat16_size_of_cluster_bytes( disk ),
        cluster = fat16_get_cluster_for_offset( disk, start_cluster, offset );
    if( cluster < 0 ) { res = cluster; goto out; }

    // calculate where & how-much to read
    int offset_from_cluster = offset % size_of_cluster_bytes,
        start_sector = fat16_cluster_to_sector( disk->fs_private, cluster ),
        start_pos = (start_sector * disk->sector_size) + offset_from_cluster,
        total_to_read = total > size_of_cluster_bytes ? size_of_cluster_bytes : total;

    // seek & read the cluster
    if( (res = diskstreamer_seek( stream, start_pos )) < 0 ) goto out;
    if( (res = diskstreamer_read( stream, out, total_to_read )) < 0 ) goto out;
    total-=total_to_read;
    
    // read next cluster
    if( total > 0 )
        res = fat16_read_internal_from_stream( disk, stream, cluster, offset + total_to_read, total, out + total_to_read );

out:
    return res;
}

static int fat16_read_internal( struct disk* disk, int starting_cluster, int offset, int total, void* out ) {
    struct fat_private* fs_private = disk->fs_private;
    struct disk_stream* stream = fs_private->cluster_read_stream;
    return fat16_read_internal_from_stream( disk, stream, starting_cluster, offset, total, out );
}

void fat16_free_directory( struct fat_directory* directory ) {
    if( !directory ) return;
    if( directory->item ) kfree( directory->item );
    kfree( directory );
}

void fat16_fat_item_free( struct fat_item* item ) {
    if( !item ) return;
    if( FAT_ITEM_TYPE_DIRECTORY == item->type ) fat16_free_directory( item->directory );
    else if( FAT_ITEM_TYPE_FILE == item->type ) kfree( item->item );
    // TODO: otherwise, panic!
    kfree( item );
}

// loads the fat_directory gives the fat_directory_item that points to it
struct fat_directory* fat16_load_fat_directory( struct disk* disk, struct fat_directory_item* item ) {
    // sanity check arguemnts
    if( !(FAT_FILE_SUBDIRECTORY & item->attribute ) ) return NULL; // -EINVARG
    
    // allocate the directory
    struct fat_directory* directory = kzalloc( sizeof( struct fat_directory ) );
    if( !directory ) return NULL; // -ENOMEM

    // get directory cluster, sector, total_items
    int cluster = fat16_get_first_cluster( item );
    directory->total = fat16_get_total_items_for_directory( disk, fat16_cluster_to_sector( disk->fs_private, cluster ) );
    
    // allocate directory items
    int res = 0, directory_size = directory->total * sizeof( struct fat_directory_item );
    if( !(directory->item = kzalloc( directory_size )) ) { res = -ENOMEM; goto out; }
    
    // read directory from disk
    if( 0 != (res = fat16_read_internal( disk, cluster, 0, directory_size, directory->item )) ) goto out;

out:
    if( 0 != res ) fat16_free_directory( directory );
    return directory;
}

// allocates & initialized as new fat item (our in-memory representation) from a directory item (on-disk representation)
struct fat_item* fat16_new_fat_item_for_directory_item( struct disk* disk, struct fat_directory_item* item ) {
    // allocate fat_item
    struct fat_item* f_item = kzalloc( sizeof( struct fat_item ) );
    if( !f_item ) return NULL;

    // check if item is a subdirectory
    if( FAT_FILE_SUBDIRECTORY & item->attribute ) {
        f_item->type = FAT_ITEM_TYPE_DIRECTORY;
        f_item->directory = fat16_load_fat_directory( disk, item );
    } else { // note: this 'else' added here, was probably a bug with lecture 52
        f_item->type = FAT_ITEM_TYPE_FILE;
        f_item->item = kheap_clone( item, sizeof( struct fat_directory_item ) ); // clone so caller can release item
    }
    return f_item;
}

// looks through a directory to find the FAT16 entry we want
struct fat_item* fat16_find_item_in_directory( struct disk* disk, struct fat_directory* directory, const char* name ) {
    char tmp_filename[PEACHOS_MAX_PATH];
    for( int i = 0; i < directory->total; i++ ) {
        // get full filename relative to directory
        fat16_get_full_relative_filename( &directory->item[i], tmp_filename, sizeof( tmp_filename ) );

        // check if we found it
        if( 0 == istrncmp( tmp_filename, name, sizeof( tmp_filename ) ) )
            return fat16_new_fat_item_for_directory_item( disk, &directory->item[i] );
    }
    return NULL;
}

// takes a path and returns the corresponding FAT16 directory entry
// note: I completely refactored this vs. lecture 52
struct fat_item* fat16_get_directory_entry( struct disk* disk, struct path_part* path ) {
    struct fat_private* fat_private = disk->fs_private;
    struct fat_directory* directory = &fat_private->root_directory;
    struct fat_item* item = NULL;

    // resolve path
    while( path ) {
        // move to next_item
        struct fat_item* next_item = fat16_find_item_in_directory( disk, directory, path->part );
        fat16_fat_item_free( item );
        if( !next_item ) return NULL;
        item = next_item;
        directory = item->directory;
        path = path->next;

        // if item is not a directory, no reason to continue on path
        if( FAT_ITEM_TYPE_DIRECTORY != item->type ) break;
    }

    // done
    if( path ) {
        fat16_fat_item_free( item );
        return NULL; // if any of the 'path' remains, then we didn't fully resolve the path
    }
    return item;
}

// takes a path and returns a file descriptor TODO: how does callee know if there's an error?
void* fat16_open( struct disk* disk, struct path_part* path, FILE_MODE mode ) {
    // read-only filesystem
    if( FILE_MODE_READ != mode ) return ERROR(-ERDONLY);
    
    // allocate a new file descriptor
    struct fat_file_descriptor* descriptor = kzalloc( sizeof( struct fat_file_descriptor ) );
    if( !descriptor ) return ERROR(-ENOMEM);

    // set descriptor position & item
    descriptor->pos = 0;
    descriptor->item = fat16_get_directory_entry( disk, path );
    if( !descriptor->item ) {
        kfree( descriptor );
        return ERROR( -EIO );
    }
    return descriptor;
}

int fat16_read( struct disk* disk, void* descriptor, uint32_t size, uint32_t nmemb, char* out ) {
    // get offset for file
    struct fat_file_descriptor* fat_desc = descriptor;
    struct fat_directory_item* item = fat_desc->item->item;
    int offset = fat_desc->pos;
    
    // read 
    for( uint32_t i = 0; i < nmemb; i++ ) {
        int res = fat16_read_internal( disk, fat16_get_first_cluster( item ), offset, size, out );
        if( res < 0 ) return res;
        out+=size;
        offset+=size;
    }
    return nmemb;
}
