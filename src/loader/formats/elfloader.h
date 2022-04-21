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
void* elf_physical_base( struct elf_file* file );
void* elf_physical_end( struct elf_file* file );
void* elf_program_header_physical_address( struct elf_file* file, struct elf32_phdr* header );

// properties
void* elf_memory( struct elf_file* file );
struct elf_header* elf_header( struct elf_file* file );
struct elf32_shdr* elf_section_header( struct elf_header* header, int index ); // section header
struct elf32_phdr* elf_program_header( struct elf_header* header, int index ); // program header
