#include "fat16.h"
#include "status.h"
#include "string/string.h"
#include <stddef.h>

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
