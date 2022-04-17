#include "elfloader.h"
#include "fs/file.h"
#include "status.h"
#include <stdbool.h>
#include "memory/memory.h"
#include "memory/heap/kheap.h"
#include "string/string.h"
#include "memory/paging/paging.h"
#include "kernel.h"
#include "config.h"

// elf signature
const char* elf_signature = {0x7f, 'E', 'L', 'F'};

// -- CHECKS --

// checks for valid ELF signature
static bool elf_valid_signature( void* buffer ) {
    return 0 == memcmp( buffer, (void*)elf_signature, sizeof( elf_signature ) );
}

// ensures instructions are 32-bit or not specified
static bool elf_valid_class( struct elf_header* header ) {
    unsigned char class = header->e_ident[EI_CLASS];
    return ELFCLASS32 == class || ELFCLASSNONE == class;
}

// ensures data is in little endian format or not specified
static bool elf_valid_encoding( struct elf_header* header ) {
    unsigned char data = header->e_ident[EI_DATA];
    return ELFDATA2LSB == data || ELFDATANONE == data;
}

// checks that its an executable file (not a shared lib), and also that the entry point is our OS's program virtual address
static bool elf_is_executable( struct elf_header* header ) {
    return header->e_type == ET_EXEC && header->e_entry >= PEACHOS_PROGRAM_VIRTUAL_ADDRESS;
}

// ensures elf has a program header
static bool elf_has_program_header( struct elf_header* header ) { return 0 != header->e_phoff; }

// validate the ELF file's signature, class, encoding & program header TODO: what about checking if the file is executable?
int elf_validate_loaded( struct elf_header* header ) {
    return (elf_valid_signature( header ) &&
            elf_valid_class( header ) &&
            elf_valid_encoding( header ) &&
            elf_has_program_header( header )) ? PEACHOS_ALL_OK : -EINVARG;
}

// -- PROPERTIES --

void* elf_memory( struct elf_file* file ) { return file->elf_memory; }

// elf header
struct elf_header* elf_header( struct elf_file* file ) { return file->elf_memory; }

// elf section header table
struct elf32_shdr* elf_sheader( struct elf_header* header ) {
    return (struct elf32_shdr*)( (int)header + header->e_shoff );
}

// elf program header table (optional)
struct elf32_phdr* elf_pheader(struct elf_header* header) {
    return 0 != header->e_phoff ? (struct elf32_phdr*)( (int)header + header->e_phoff ) : 0;
}

// elf header (program or section) @ index
struct elf32_phdr* elf_program_header( struct elf_header* header, int index ) { return &elf_pheader( header )[index]; }
struct elf32_shdr* elf_section_header( struct elf_header* header, int index ) { return &elf_sheader( header )[index]; }

// points to string table section
char* elf_str_table( struct elf_header* header ) {
    struct elf32_shdr* stringtable_section_header = elf_section_header( header, header->e_shstrndx );
    return (char*)header + stringtable_section_header->sh_offset;
}

void* elf_virtual_base( struct elf_file* file ) { return file->virtual_base_address; }
void* elf_virtual_end( struct elf_file* file ) { return file->virtual_end_address; }
void* elf_phys_base( struct elf_file* file ) { return file->physical_base_address; }
void* elf_phys_end( struct elf_file* file ) { return file->physical_end_address; }
