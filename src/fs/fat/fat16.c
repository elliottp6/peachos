#include "fat16.h"
#include "status.h"
#include "string/string.h"
#include "disk/disk.h"
#include "disk/streamer.h"
#include "memory/memory.h"
#include "memory/heap/kheap.h"
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

// fat16 strings terminate either with null or with space, so we must convert to null-terminated
void fat16_to_proper_string( char** out, const char* in ) {
    while( *in != 0x00 && *in != 0x20 ) { **out = *in; *out+=1; in+=1; }
    if( *in == 0x20 ) **out = 0x00;
}

void fat16_get_full_relative_filename( struct fat_directory_item* item, char* out, int max_len ) {
    // clear output string
    memset( out, 0, max_len );
    
    // write filename to string
    char *out_tmp = out;
    fat16_to_proper_string( &out_tmp, (const char*)item->filename );
    
    // append extension (if extension is not empty)
    if( item->ext[0] && item->ext[0] != 0x20 ) {
        *out_tmp++ = '.';
        fat16_to_proper_string( &out_tmp, (const char*)item->ext );
    }
}

// TODO: this seems odd... why are we not casting to uint32_t and then shifting the high bits before or-ing?
static uint32_t fat16_get_first_cluster( struct fat_directory_item* item ) {
    return (item->high_16_bits_first_cluster) | item->low_16_bits_first_cluster;
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
    int res = 0;
    int size_of_cluster_bytes = fat16_size_of_cluster_bytes( disk );
    int cluster_count = offset / size_of_cluster_bytes;
    
    // go through each fat entry to find pointer to next cluster, until we reach offset
    for( int i = 0; i < cluster_count; i++ ) {
        // get entry
        int entry = fat16_get_fat_entry( disk, cluster );

        // check if sector 'last', marked as bad, is reserved, or is zero
        if( 0xFF8 == entry || 0xFFF == entry || // last entry in the file
            PEACHOS_FAT16_BAD_SECTOR == entry || // bad sector
            0xFF0 == entry || 0xFF6 == entry || // reserved sector
            0x000 == entry ) { // null entry
            res = -EIO;
            goto out;
        }

        // otherwise, 'entry' points to the next cluster
        cluster = entry;
    }

    res = cluster;
out:
    return res;
}

static int fat16_read_internal_from_stream( struct disk* disk, struct disk_stream* stream, int start_cluster, int offset, int total, void* out ) {
    // get cluster
    int res = 0;
    int size_of_cluster_bytes = fat16_size_of_cluster_bytes( disk );
    int cluster = fat16_get_cluster_for_offset( disk, start_cluster, offset );
    if( cluster < 0 ) { res = cluster; goto out; }

    // calculate where & how-much to read
    int offset_from_cluster = offset % size_of_cluster_bytes;
    int start_sector = fat16_cluster_to_sector( disk->fs_private, cluster );
    int start_pos = (start_sector * disk->sector_size) * offset_from_cluster; // TODO: not + ???
    int total_to_read = total > size_of_cluster_bytes ? size_of_cluster_bytes : total;

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

struct fat_directory* fat16_load_fat_directory( struct disk* disk, struct fat_directory_item* item ) {
    int res = 0;
    struct fat_directory* directory = NULL;
    struct fat_private* fat_private = disk->fs_private;
    
    // ensure 'item' is a directory
    if( !(FAT_FILE_SUBDIRECTORY & item->attribute ) ) { res = -EINVARG; goto out; }

    // allocate the directory
    directory = kzalloc( sizeof( struct fat_directory ) );
    if( !directory ) { res = -ENOMEM; goto out; }

    // get directory cluster, sector, and total_items
    int cluster = fat16_get_first_cluster( item );
    int cluster_sector = fat16_cluster_to_sector( fat_private, cluster );
    int total_items = fat16_get_total_items_for_directory( disk, cluster_sector );
    directory->total = total_items;
    
    // allocate directory items
    int directory_size = total_items * sizeof( struct fat_directory_item );
    if( !(directory->item = kzalloc( directory_size )) ) { res = -ENOMEM; goto out; }
    
    // read directory from disk
    if( 0 != (res = fat16_read_internal( disk, cluster, 0, directory_size, directory->item )) ) goto out;

out:
    if( 0 != res ) fat16_free_directory( directory );
    return directory;
}

struct fat_item* fat16_new_fat_item_for_directory_item( struct disk* disk, struct fat_directory_item* item ) {
    // allocate fat_item
    struct fat_item* f_item = kzalloc( sizeof( struct fat_item ) );
    if( !f_item ) return NULL;

    // check if item is a subdirectory
    if( FAT_FILE_SUBDIRECTORY & item->attribute ) {
        f_item->directory = fat16_load_fat_directory( disk, item );
        f_item->type = FAT_ITEM_TYPE_DIRECTORY;
        // TODO: return here before type if overwritten?
    }

    // note: must clone the fat_directory_item since caller may own this object
    f_item->type = FAT_ITEM_TYPE_FILE;
    f_item->item = kheap_clone( item, sizeof( struct fat_directory_item ) );
    return f_item;
}

struct fat_item* fat16_find_item_in_directory( struct disk* disk, struct fat_directory* directory, const char* name ) {
    struct fat_item* f_item = NULL;
    char tmp_filename[PEACHOS_MAX_PATH];
    for( int i = 0; i < directory->total; i++ ) {
        // get full filename relative to directory
        fat16_get_full_relative_filename( &directory->item[i], tmp_filename, sizeof( tmp_filename ) );

        // check if we found it
        if( 0 == istrncmp( tmp_filename, name, sizeof( tmp_filename ) ) ) {
            f_item = fat16_new_fat_item_for_directory_item( disk, &directory->item[i] );
            // TODO: break?
        }
    }
    return f_item;
}

struct fat_item* fat16_get_directory_entry( struct disk* disk, struct path_part* path ) {
    // result
    struct fat_item* current_item = NULL;

    // get root item
    struct fat_private* fat_private = disk->fs_private;
    struct fat_item* root_item = fat16_find_item_in_directory( disk, &fat_private->root_directory, path->part );
    if( !root_item ) goto out;

    // process rest of path
    struct path_part* next_part = path->next;
    current_item = root_item;
    while( next_part ) {
        // if this is a file, we're done
        if( FAT_ITEM_TYPE_DIRECTORY != current_item->type ) { current_item = NULL; break; }

        // otherwise, continue searching in the next directory
        struct fat_item* tmp_item = fat16_find_item_in_directory( disk, current_item->directory, next_part->part );
        fat16_fat_item_free( current_item );
        current_item = tmp_item;
        next_part = next_part->next;
    }

out:
    return current_item;
}

void* fat16_open( struct disk* disk, struct path_part* path, FILE_MODE mode ) {
    // read-only filesystem TODO: how does callee know this is an error???
    if( FILE_MODE_READ != mode ) return ERROR(-ERDONLY);
    
    // allocate a new file descriptor TODO: failure paths need to deallocate
    struct fat_file_descriptor* descriptor = kzalloc( sizeof( struct fat_file_descriptor ) );
    if( !descriptor ) return ERROR(-ENOMEM);

    // set descriptor position & item
    descriptor->pos = 0;
    descriptor->item = fat16_get_directory_entry( disk, path );
    if( !descriptor->item ) return ERROR( -EIO );
    return descriptor;
}
