// Executable & Linking Format
// https://refspecs.linuxfoundation.org/elf/elf.pdf
// https://www.udemy.com/course/developing-a-multithreaded-kernel-from-scratch/learn/lecture/24029680
#pragma once
#include <stdint.h>
#include <stddef.h>

// constants
#define PF_X 0x01
#define PF_W 0x02
#define PF_R 0x04

// segment types
#define PT_NULL 0
#define PT_LOAD 1 // loadable segment
#define PT_DYNAMIC 2
#define PT_INTERP 3
#define PT_NOTE 4
#define PT_SHLIB 5
#define PT_PHDR 6

#define SHT_NULL 0
#define SHT_PROGBITS 1
#define SHT_SYMTAB 2
#define SHT_STRTAB 3
#define SHT_RELA 4
#define SHT_HASH 5
#define SHT_DYNAMIC 6
#define SHT_NOTE 7
#define SHT_NOBITS 8
#define SHT_REL 9
#define SHT_SHLIB 10
#define SHT_DYNSYM 11
#define SHT_LOPROC 12
#define SHT_HIPROC 13
#define SHT_LOUSER 14
#define SHT_HIUSER 15

// type of object file
#define ET_NONE 0 // type not specified
#define ET_REL 1 // relocatable
#define ET_EXEC 2 // executable
#define ET_DYN 3 // dynamic link library
#define ET_CORE 4 // core file

#define EI_NIDENT 16 // number of identity bytes used to define this as ELF and its class (i.e. subtype)
#define EI_CLASS 4 // index of 'class' byte within the identity bytes
#define EI_DATA 5 // index of 'data' byte within identity bytes

#define ELFCLASSNONE 0
#define ELFCLASS32 1
#define ELFCLASS64 2

// data encoding
#define ELFDATANONE 0 // no encoding specified
#define ELFDATA2LSB 1 // little endian encoding (intel's default)
#define ELFDATA2MSB 2 // big endian encoding (we won't support this, but we could in theory do a conversion or something, right?)

#define SHN_UNDEF 0

// data types
typedef uint16_t elf32_half;
typedef uint32_t elf32_word;
typedef int32_t elf32_sword;
typedef uint32_t elf32_addr;
typedef int32_t elf32_off;

// elf header
struct elf_header {
    unsigned char e_ident[EI_NIDENT]; // marks this file as an ELF file type, along with it's class (i.e. subtype)
    elf32_half e_type; // type of object file (relocatable, executable, etc.)
    elf32_half e_machine; // architecture (EM_386 is what we want)
    elf32_word e_version; // object file version
    elf32_addr e_entry; // virtual address to start execution
    elf32_off e_phoff; // offset to 0th program header
    elf32_off e_shoff; // offset to 0th section header
    elf32_word e_flags; // processor-specific flags
    elf32_half e_ehsize; // size of the elf header
    elf32_half e_phentsize; // sizeof program header
    elf32_half e_phnum; // # program headers
    elf32_half e_shentsize; // sizeof section header
    elf32_half e_shnum; // # section headers
    elf32_half e_shstrndx; // section header table index of the string table
} __attribute__((packed));

// program header
struct elf32_phdr {
    elf32_word p_type;
    elf32_off p_offset;
    elf32_addr p_vaddr;
    elf32_addr p_paddr;
    elf32_word p_filesz;
    elf32_word p_memsz;
    elf32_word p_flags;
    elf32_word p_align;
} __attribute__((packed));

// section header
struct elf32_shdr {
    elf32_word sh_name;
    elf32_word sh_type;
    elf32_word sh_flags;
    elf32_addr sh_addr;
    elf32_off sh_offset; // 
    elf32_word sh_size;
    elf32_word sh_link;
    elf32_word sh_info;
    elf32_word sh_addralign;
    elf32_word sh_entsize;
} __attribute__((packed));

// dynamic section structure
struct elf32_dyn {
    elf32_sword d_tag;
    union {
        elf32_word d_val;
        elf32_addr d_ptr;
    } d_un;
} __attribute__((packed));

// symbol table entry
struct elf32_sym {
    elf32_word st_name;
    elf32_addr st_value;
    elf32_word st_size;
    unsigned char st_info;
    unsigned char st_other;
    elf32_half st_shndx;
} __attribute__((packed));

// functions
uint32_t elf_get_entry( struct elf_header* elf_header );
void* elf_get_entry_ptr( struct elf_header* elf_header );
