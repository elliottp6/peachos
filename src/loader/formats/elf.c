#include "elf.h"

uint32_t elf_get_entry( struct elf_header* elf_header ) { return elf_header->e_entry; }
void* elf_get_entry_ptr( struct elf_header* elf_header ) { return (void*)elf_header->e_entry; }

