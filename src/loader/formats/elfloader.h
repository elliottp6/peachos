#pragma once
#include <stdint.h>
#include <stddef.h>
#include "elf.h"
#include "config.h"

// types
struct elf_file {
    char filename[PEACHOS_MAX_PATH];
    int in_memory_size; // how large the elf file is when loaded into memory
    void* elf_memory; // physical memory address where ELF file is loaded
    void* virtual_base_address; // the lowest address of virtual memory
    void* virtual_end_address;
    void* physical_base_address;
    void* physical_end_address;
};

// functions
int elf_load( const char* filename, struct elf_file** file_out );
void elf_close( struct elf_file* file );
void* elf_virtual_base( struct elf_file* file );
void* elf_virtual_end( struct elf_file* file );
void* elf_phys_base( struct elf_file* file );
void* elf_phys_end( struct elf_file* file );
