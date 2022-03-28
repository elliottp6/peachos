#include "file.h"
#include "config.h"
#include "memory/memory.h"
#include "memory/heap/kheap.h"
#include "status.h"
#include "kernel.h"
#include "fat/fat16.h"
#include "disk/disk.h"
#include "string/string.h"
#include <stddef.h>

struct filesystem* filesystems[PEACHOS_MAX_FILESYSTEMS];
struct file_descriptor* file_descriptors[PEACHOS_MAX_FILE_DESCRIPTORS];

static struct filesystem** fs_get_free_filesystem() {
    int i;
    for( i = 0; i < PEACHOS_MAX_FILESYSTEMS; i++ ) if( 0 == filesystems[i] ) return &filesystems[i];
    return NULL;
}

void fs_insert_filesystem( struct filesystem* filesystem ) {
    // TODO: panic if filesystem is null: if( NULL == filesystem ) 

    // get free slot for filesystem
    struct filesystem** fs = fs_get_free_filesystem();

    // panic (implement me later as a function) if there is not a free filesystem
    if( !fs ) {
        print( "Problem inserting filesystem\n" );
        while( 1 ) {}
    }

    // insert filesystem into the filesystem array
    *fs = filesystem;
}

// load static filesystems (i.e. those that are built into the kernel, not dynamically loaded)
static void fs_static_load() {
    fs_insert_filesystem( fat16_init() );
}

void fs_load() {
    memset( filesystems, 0, sizeof( filesystems ) );
    fs_static_load();
}

void fs_init() {
    memset( file_descriptors, 0, sizeof( file_descriptors ) );
    fs_load();
}

static int fs_get_free_file_descriptor_slot_index() {
    for( int i = 0; i < PEACHOS_MAX_FILE_DESCRIPTORS; i++ ) if( 0 == file_descriptors[i] ) return i;
    return -1;
}

static int file_new_descriptor( struct file_descriptor** desc_out ) {
    // get free slot index for file descriptor
    int i = fs_get_free_file_descriptor_slot_index();
    if( i < 0 ) { *desc_out = NULL; return -ENOMEM; }

    // create file descriptor and put it into slot
    struct file_descriptor* desc = kzalloc( sizeof( struct file_descriptor ) );
    desc->index = i + 1; // descriptor indices start @ 1
    file_descriptors[i] = desc;
    *desc_out = desc;
    return 0;
}

static struct file_descriptor* file_get_descriptor( int fd ) {
    if( fd <= 0 || fd > PEACHOS_MAX_FILE_DESCRIPTORS ) return NULL; // bounds check (NOTE: bugfix from lecture 43: I don't believe we want >= here, we just want >, because MAX_FILE_DESCRIPTOR - 1 is a valid slot in the array)
    return file_descriptors[fd - 1];
}

struct filesystem* fs_resolve( struct disk* disk ) {
    for( int i = 0; i < PEACHOS_MAX_FILESYSTEMS; i++ )
        if( NULL != filesystems[i] && 0 == filesystems[i]->resolve( disk ) ) return filesystems[i];
    return NULL;
}

FILE_MODE file_get_mode_by_string( const char* str ) {
    if( 0 == strncmp( str, "r", 1 ) ) return FILE_MODE_READ;
    else if( 0 == strncmp( str, "w", 1 ) ) return FILE_MODE_WRITE;
    else if( 0 == strncmp( str, "a", 1 ) ) return FILE_MODE_APPEND;
    return FILE_MODE_INVALID;
}

int fopen( const char* filename, const char* mode_string ) {
    // parse path
    int res = 0;
    struct path_root* root_path = pathparser_parse( filename, NULL );
    if( !root_path || !root_path->first ) { res = -EINVARG; goto out; }

    // get disk and check for filesystem
    struct disk* disk = disk_get( root_path->drive_no );
    if( !disk || !disk->filesystem ) { res = -EIO; goto out; }
    
    // determine file mode
    FILE_MODE mode = file_get_mode_by_string( mode_string );
    if( FILE_MODE_INVALID == mode ) { res = -EINVARG; goto out; }

    // tell filesystem to open the file
    void* descriptor_private_data = disk->filesystem->open( disk, root_path->first, mode );
    if( ISERR( descriptor_private_data ) ) { res = ERROR_I( descriptor_private_data ); goto out; }

    /// create & initialize a new file descriptor
    struct file_descriptor* desc = 0;
    res = file_new_descriptor( &desc );
    if( res < 0 ) goto out;
    desc->filesystem = disk->filesystem;
    desc->private_data = descriptor_private_data;
    desc->disk = disk;
    res = desc->index;

out:
    // TODO: if( root_path ) pathparser_free( root_path ); // <-- this seems VERY important, but in the lecture he does not clear the memory
    if( res < 0 ) { 
        // TODO: clear memory... isn't there more that needs to be done here?
        return 0; // fopen shouldn't return negative values
    }
    return res;
}

int fread( void* buffer, uint32_t size, uint32_t nmemb, int fd ) {
    // sanity check args
    if( 0 == size || 0 == nmemb || fd < 1 ) return -EINVARG;

    // get file descriptor
    struct file_descriptor* desc = file_get_descriptor( fd );
    if( !desc ) return -EINVARG;

    // call the filesystem to do the read
    return desc->filesystem->read( desc->disk, desc->private_data, size, nmemb, (char*)buffer );
}
