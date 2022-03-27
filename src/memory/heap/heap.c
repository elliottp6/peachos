#include "heap.h"
#include "kernel.h"
#include "status.h"
#include "memory/memory.h"
#include <stdbool.h>

static int heap_validate_table( void* start, void* end, struct heap_table* table ) {
    size_t table_size = (size_t)(end - start);
    size_t total_blocks = table_size / PEACHOS_HEAP_BLOCK_SIZE;
    if( table->total != total_blocks ) return -EINVARG;
    return 0;
}

static bool heap_validate_alignment( void* p ) { return 0 == ((uint32_t)p % PEACHOS_HEAP_BLOCK_SIZE); }

int heap_create( struct heap* heap, void* start, void* end, struct heap_table* table ) {
    int res = 0;

    // validate pointer alignment
    if( !heap_validate_alignment( start ) || !heap_validate_alignment( end ) ) {
        res = -EINVARG;
        goto out;
    }

    // clear the heap struct
    memset( heap, 0, sizeof( struct heap ) );
    heap->start = start;
    heap->table = table;

    // validate the table
    res = heap_validate_table( start, end, table );
    if( res < 0 ) goto out;

    // clear the heap table entries
    size_t table_size = sizeof( HEAP_BLOCK_TABLE_ENTRY ) * table->total;
    memset( table->entries, HEAP_BLOCK_TABLE_ENTRY_FREE, table_size );

out:
    return res;
}

// align value rounded up
static uint32_t heap_align_value_to_upper( uint32_t value ) {
    uint32_t r = value % PEACHOS_HEAP_BLOCK_SIZE;
    if( 0 == r ) return value;
    return value - r + PEACHOS_HEAP_BLOCK_SIZE;
}

static int heap_get_entry_type( HEAP_BLOCK_TABLE_ENTRY entry ) { return entry & 0xF; }

int heap_get_start_block( struct heap* heap, uint32_t total_blocks ) {
    struct heap_table* table = heap->table;
    int bc = 0, bs = -1; // current block, block_start
    for( size_t i = 0; i < table->total; i++ ) {
        // check for reset
        if( HEAP_BLOCK_TABLE_ENTRY_FREE != heap_get_entry_type( table->entries[i] ) ) {
            bc = 0; bs = -1; continue;
        }
        
        // check for start
        if( -1 == bs ) bs = i;

        // check for end
        if( ++bc == total_blocks ) break;
    }

    // return either no-memory error or start block
    if( -1 == bs ) return -ENOMEM;
    return bs;
}

void* heap_block_to_address( struct heap* heap, int block ) {
    return heap->start + (block * PEACHOS_HEAP_BLOCK_SIZE);
}

void heap_mark_blocks_taken( struct heap* heap, int start_block, int total_blocks ) {
    int end_block = start_block + total_blocks - 1;
    HEAP_BLOCK_TABLE_ENTRY entry = HEAP_BLOCK_TABLE_ENTRY_TAKEN | HEAP_BLOCK_IS_FIRST;
    if( total_blocks > 1 ) entry|= HEAP_BLOCK_HAS_NEXT;

    // mark blocks
    for( int i = start_block; i <= end_block; i++ ) {
        heap->table->entries[i] = entry;
        entry = HEAP_BLOCK_TABLE_ENTRY_TAKEN;
        if( end_block - 1 != i ) entry|=HEAP_BLOCK_HAS_NEXT;
    }
}

void* heap_malloc_blocks( struct heap* heap, uint32_t total_blocks ) {
    void* address = 0;
    int start_block = heap_get_start_block( heap, total_blocks );
    if( start_block < 0 ) goto out;

    address = heap_block_to_address( heap, start_block );

    // mark the blocks as taken
    heap_mark_blocks_taken( heap, start_block, total_blocks );

out:
    return address;
}

void* heap_malloc( struct heap* heap, size_t size ) {
    size_t aligned_size = heap_align_value_to_upper( size );
    uint32_t total_blocks = aligned_size / PEACHOS_HEAP_BLOCK_SIZE;
    return heap_malloc_blocks( heap, total_blocks );
}

int heap_address_to_block( struct heap* heap, void* address ) {
    return ((int)(address - heap->start)) / PEACHOS_HEAP_BLOCK_SIZE;
}

void heap_mark_blocks_free( struct heap* heap, int start_block ) {
    struct heap_table* table = heap->table;
    for( int i = start_block; i < (int)table->total; i++ ) {
        HEAP_BLOCK_TABLE_ENTRY entry = table->entries[i];
        table->entries[i] = HEAP_BLOCK_TABLE_ENTRY_FREE;
        if( !( HEAP_BLOCK_HAS_NEXT & entry ) ) break;
    }
}

void heap_free( struct heap* heap, void* p ) {
    heap_mark_blocks_free( heap, heap_address_to_block( heap, p ) );
}

void* heap_clone( struct heap* heap, void* buffer, size_t size ) {
    void* clone = heap_malloc( heap, size );
    if( !clone ) return NULL;
    memcpy( clone, buffer, size );
    return clone;
}
