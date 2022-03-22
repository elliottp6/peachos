#include "file.h"
#include "config.h"
#include "memory/memory.h"
#include "memory/heap/kheap.h"
#include "status.h"
#include "kernel.h"
#include "fat/fat16.h"
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

int fopen( const char* filename, const char* mode ) {
    return -EIO;
}
