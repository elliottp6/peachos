// virtual file system (VFS)
// https://www.udemy.com/course/developing-a-multithreaded-kernel-from-scratch/learn/lecture/23984448
#pragma once
#include <stdint.h>
#include "pparser.h"

// enums
typedef uint32_t FILE_SEED_MODE; enum { SEEK_SET, SEEK_CUR, SEEK_END };
typedef uint32_t FILE_MODE; enum { FILE_MODE_READ, FILE_MODE_WRITE, FILE_MODE_APPEND, FILE_MODE_INVALID };

// forward declaration
struct disk;

// function pointer types
typedef int (*FS_RESOLVE_FUNCTION)( struct disk* disk ); // can the VHS read this filesystem?
typedef void*(*FS_OPEN_FUNCTION)( struct disk* disk, struct path_part* path, FILE_MODE mode );
typedef int (*FS_READ_FUNCTION)( struct disk* disk, void* private, uint32_t size, uint32_t nmemb, char* out );
typedef int (*FS_SEEK_FUNCTION)( void* private, uint32_t offset, FILE_SEED_MODE mode );

// types
struct filesystem { // filesystem interface
    FS_RESOLVE_FUNCTION resolve; // should return 0 if the provided disk is using its filesystem
    FS_OPEN_FUNCTION open;
    FS_READ_FUNCTION read;
    FS_SEEK_FUNCTION seek;
    char name[20];
};

struct file_descriptor {
    int index;
    struct filesystem* filesystem;
    void* private; // private data for internal file descriptor
    struct disk* disk;
};

// functions
void fs_init(); // initialize the filesystems
int fopen( const char* filename, const char* mode_string ); // open a file given only a path
int fread( void* buffer, uint32_t size, uint32_t nmemb, int fd );
int fseek( int fd, int offset, FILE_SEED_MODE whence );
void fs_insert_filesystem( struct filesystem* filesystem ); // add filesystem to the system
struct filesystem* fs_resolve( struct disk* disk ); // determine the filesystem on this disk
