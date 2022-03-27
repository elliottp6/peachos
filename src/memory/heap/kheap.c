#include "kheap.h"
#include "heap.h"
#include "config.h"
#include "kernel.h"
#include "memory/memory.h"

struct heap kernel_heap;
struct heap_table kernel_heap_table;

void kheap_init() {
    // initialize kernel_heap_table
    kernel_heap_table.entries = (HEAP_BLOCK_TABLE_ENTRY*)PEACHOS_HEAP_TABLE_ADDRESS;
    kernel_heap_table.total = PEACHOS_HEAP_SIZE_BYTES / PEACHOS_HEAP_BLOCK_SIZE;
    
    // initialize kernel_heap
    void* end = (void*)(PEACHOS_HEAP_ADDRESS + PEACHOS_HEAP_SIZE_BYTES);
    int res = heap_create( &kernel_heap, (void*)PEACHOS_HEAP_ADDRESS, end, &kernel_heap_table );
    
    // check for error
    if( res < 0 ) print( "failed to create kernel heap\n" );
}

void* kmalloc( size_t size ) { return heap_malloc( &kernel_heap, size ); }

void* kzalloc( size_t size ) {
    void* p = kmalloc( size );
    if( !p ) return NULL;
    memset( p, 0, size );
    return p;
}

void kfree( void* p ) { heap_free( &kernel_heap, p ); }

void* kheap_clone( void* buffer, size_t size ) { return heap_clone( &kernel_heap, buffer, size ); }
