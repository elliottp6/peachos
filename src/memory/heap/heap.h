// https://www.udemy.com/course/developing-a-multithreaded-kernel-from-scratch/learn/lecture/23973026
#pragma once
#include <stdint.h>
#include <stddef.h>
#include "config.h"

#define HEAP_BLOCK_TABLE_ENTRY_FREE  0x00
#define HEAP_BLOCK_TABLE_ENTRY_TAKEN 0x01
#define HEAP_BLOCK_HAS_NEXT 0b10000000
#define HEAP_BLOCK_IS_FIRST 0b01000000

typedef uint8_t HEAP_BLOCK_TABLE_ENTRY;

struct heap_table {
    HEAP_BLOCK_TABLE_ENTRY* entries;
    size_t total;
};

struct heap {
    struct heap_table* table;
    void* start;
};

int heap_create( struct heap* heap, void* start, void* end, struct heap_table* table );
void* heap_malloc( struct heap* heap, size_t size );
void heap_free( struct heap* heap, void* p );
void* heap_clone( struct heap* heap, void* buffer, size_t size );
