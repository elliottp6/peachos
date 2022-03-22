// https://www.udemy.com/course/developing-a-multithreaded-kernel-from-scratch/learn/lecture/23983268
#pragma once
#include <stdint.h>
#include "fs/file.h"

typedef uint32_t PEACHOS_DISK_TYPE;

#define PEACHOS_DISK_TYPE_REAL 0 // represents a real, physical hard disk

struct disk {
    PEACHOS_DISK_TYPE type;
    int sector_size;
    struct filesystem* filesystem;
};

// functions
void disk_search_and_init();
struct disk* disk_get( int index );
int disk_read_block( struct disk* idisk, uint32_t lba, int total, void* buffer );
