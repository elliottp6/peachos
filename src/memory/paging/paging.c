#include "paging.h"
#include "memory/heap/kheap.h"
#include "status.h"

void paging_load_directory( uint32_t* directory );

static uint32_t* current_directory = 0;

// initialize page tables w/ the identity transformation
struct paging_4gb_chunk* paging_new_4gb( uint8_t flags ) {
    // allocate & initialize directory & tables
    uint32_t* directory = kzalloc( sizeof( uint32_t ) * PAGING_TOTAL_ENTRIES_PER_TABLE );
    for( int i = 0, offset = 0; i < PAGING_TOTAL_ENTRIES_PER_TABLE; i++, offset += PAGING_TABLE_SIZE ) {
        uint32_t* entry = kzalloc( sizeof( uint32_t ) * PAGING_TOTAL_ENTRIES_PER_TABLE );
        for( int b = 0; b < PAGING_TOTAL_ENTRIES_PER_TABLE; b++ ) entry[b] = (offset + (b * PAGING_PAGE_SIZE)) | flags;
        directory[i] = (uint32_t)entry | flags | PAGING_IS_WRITABLE;
    }

    // wrap directory in the paging_4gb_chunk struct
    struct paging_4gb_chunk* chunk_4gb = kzalloc( sizeof( struct paging_4gb_chunk ) );
    chunk_4gb->directory_entry = directory;
    return chunk_4gb;
}

void paging_switch( uint32_t* directory ) {
    paging_load_directory( directory );
    current_directory = directory;
}

uint32_t* paging_4gb_chunk_get_directory( struct paging_4gb_chunk* chunk ) { return chunk->directory_entry; }

bool paging_is_aligned( void* address ) { return 0 == ((uint32_t)address % PAGING_PAGE_SIZE); }

int paging_get_indexes( void* virtual_address, uint32_t* directory_index_out, uint32_t* table_index ) {
    // check alignment
    int res = 0;
    if( !paging_is_aligned( virtual_address ) ) { res = -EINVARG; goto out; }

    // calculate directory & table
    *directory_index_out = (uint32_t)virtual_address / PAGING_TABLE_SIZE;
    *table_index = ((uint32_t)virtual_address % PAGING_TABLE_SIZE) / PAGING_PAGE_SIZE; // remainder of directory_index rounded down to page_size

out:
    return res;
}

int paging_set( uint32_t* directory, void* virtual_address, uint32_t value ) {
    // sanity check args
    if( !paging_is_aligned( virtual_address ) ) return -EINVARG;

    // get directory_index & table_index
    uint32_t directory_index = 0, table_index = 0;
    int res = paging_get_indexes( virtual_address, &directory_index, &table_index );
    if( res < 0 ) return res;

    // get directory entry & table entry
    uint32_t entry = directory[directory_index];
    uint32_t* table = (uint32_t*)(entry & 0xFFFFF000); // get just the address part of the entry (top 20 bits)
    table[table_index] = value;
    return res;
}
