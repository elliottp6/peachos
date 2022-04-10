#include "paging.h"
#include "memory/heap/kheap.h"
#include "status.h"
#include "kernel.h"

void paging_load_directory( uint32_t* directory );

static uint32_t* current_directory = 0;

// initialize page tables w/ the identity transformation
struct paging_4gb_chunk* paging_new_4gb( uint8_t flags ) {
    // allocate & initialize directory & tables
    uint32_t* directory = kzalloc( sizeof( uint32_t ) * PAGING_TOTAL_ENTRIES_PER_TABLE );
    for( int i = 0, offset = 0; i < PAGING_TOTAL_ENTRIES_PER_TABLE; i++, offset += PAGING_TABLE_SIZE ) {
        uint32_t* entry = kzalloc( sizeof( uint32_t ) * PAGING_TOTAL_ENTRIES_PER_TABLE );
        for( int b = 0; b < PAGING_TOTAL_ENTRIES_PER_TABLE; b++ ) entry[b] = (offset + (b * PAGING_PAGE_SIZE)) | flags;
        directory[i] = (uint32_t)entry | flags | PAGING_IS_WRITEABLE;
    }

    // wrap directory in the paging_4gb_chunk struct
    struct paging_4gb_chunk* chunk_4gb = kzalloc( sizeof( struct paging_4gb_chunk ) );
    chunk_4gb->directory_entry = directory;
    return chunk_4gb;
}

void paging_switch( struct paging_4gb_chunk* directory ) {
    paging_load_directory( directory->directory_entry );
    current_directory = directory->directory_entry;
}

void paging_free_4gb( struct paging_4gb_chunk* chunk ) {
    for( int i = 0; i < PAGING_TOTAL_ENTRIES_PER_TABLE; i++ ) {
        uint32_t entry = chunk->directory_entry[i];
        uint32_t* table = (uint32_t*)(entry & 0xFFFFF000); // grab just the pointer part of the directory entry
        kfree( table );
    }
    kfree( chunk->directory_entry );
    kfree( chunk );
}

uint32_t* paging_4gb_chunk_get_directory( struct paging_4gb_chunk* chunk ) { return chunk->directory_entry; }

bool paging_is_aligned( void* address ) { return 0 == ((uint32_t)address % PAGING_PAGE_SIZE); }

int paging_get_indexes( void* virtual_address, uint32_t* directory_index_out, uint32_t* table_index_out ) {
    if( !paging_is_aligned( virtual_address ) ) return -EINVARG;
    *directory_index_out = (uint32_t)virtual_address / PAGING_TABLE_SIZE;
    *table_index_out = ((uint32_t)virtual_address % PAGING_TABLE_SIZE) / PAGING_PAGE_SIZE; // remainder of directory_index rounded down to page_size
    return 0;
}

void* paging_align_address( void* address ) { // aligns address by rounding up
    uint32_t remainder = (uint32_t)address % PAGING_PAGE_SIZE;
    if( remainder ) return (void*)((uint32_t)address + PAGING_PAGE_SIZE - remainder);
    return address;
}

int paging_map( struct paging_4gb_chunk* directory, void* virt, void* phys, int flags ) {
    // check alignment
    if( (uint32_t)virt % PAGING_PAGE_SIZE || (uint32_t)phys % PAGING_PAGE_SIZE ) return -EINVARG;
    
    // set the page
    return paging_set( directory->directory_entry, virt, (uint32_t)phys | flags );
}

// TODO: bugfix in lecture 68: the '0 != (res = paging_map'... line was '0 == (res = paging_map'...
int paging_map_range( struct paging_4gb_chunk* directory, void* virt, void* phys, int count, int flags ) {
     int res;
     for( int i = 0; i < count; i++ ) {
         if( 0 != (res = paging_map( directory, virt, phys, flags )) ) return res;
         virt += PAGING_PAGE_SIZE;
         phys += PAGING_PAGE_SIZE;
     }
     return 0;
}

int paging_map_to( struct paging_4gb_chunk* directory, void* virt, void* phys, void* phys_end, int flags ) {
    // sanity check addresses
    if( (uint32_t)virt % PAGING_PAGE_SIZE || (uint32_t)phys % PAGING_PAGE_SIZE ||
        (uint32_t)phys_end % PAGING_PAGE_SIZE || (uint32_t)phys_end < (uint32_t)phys )
        return -EINVARG;

    // calculate size of mapping
    uint32_t total_bytes = phys_end - phys, total_pages = total_bytes / PAGING_PAGE_SIZE;

    // do the mapping
    return paging_map_range( directory, virt, phys, total_pages, flags );
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

uint32_t paging_get( uint32_t* directory, void* virtual_address ) {
    // get table indices
    uint32_t directory_index = 0, table_index = 0;
    if( paging_get_indexes( virtual_address, &directory_index, &table_index ) < 0 )
        panic( "cannot get page table index for unaligned address\n" );

    // get entry and get table pointer
    uint32_t entry = directory[directory_index], *table = (uint32_t*)(entry & 0xFFFFF000);

    // lookup address in table
    return table[table_index];
}
