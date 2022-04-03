// https://www.udemy.com/course/developing-a-multithreaded-kernel-from-scratch/learn/lecture/23983188
// https://wiki.osdev.org/Paging
#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// page table entry bits (where entry = 32-bit)
#define PAGING_CACHE_DISABLED  0b00010000
#define PAGING_WRITE_THROUGH   0b00001000
#define PAGING_ACCESS_FROM_ALL 0b00000100 // allows access from all privilege levels
#define PAGING_IS_WRITEABLE     0b00000010
#define PAGING_IS_PRESENT      0b00000001

#define PAGING_TOTAL_ENTRIES_PER_TABLE 1024
#define PAGING_PAGE_SIZE 4096
#define PAGING_TABLE_SIZE 4194304 // PAGING_TOTAL_ENTRIES_PER_TABLE * PAGING_PAGE_SIZE

// types
struct paging_4gb_chunk { uint32_t* directory_entry; };

// functions
uint32_t* paging_4gb_chunk_get_directory( struct paging_4gb_chunk* chunk );
struct paging_4gb_chunk* paging_new_4gb( uint8_t flags );
void paging_free_4gb( struct paging_4gb_chunk* chunk );
void paging_switch( uint32_t* directory );
void enable_paging(); // warning: must create page tables & switch to a given directory BEFORE enabling paging (or else, kernel panic)

// checks is a 'address' is aligned to page boundary
bool paging_is_aligned( void* address );

// page mapping
int paging_set( uint32_t* directory, void* virtual_address, uint32_t value );
int paging_map( uint32_t* directory, void* virt, void* phys, int flags );
int paging_map_range( uint32_t* directory, void* virt, void* phys, int count, int flags );
int paging_map_to( uint32_t* directory, void* virt, void* phys, void* phys_end, int flags );
void* paging_align_address( void* address );
