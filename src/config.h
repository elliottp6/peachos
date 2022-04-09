#pragma once

#define KERNEL_CODE_SELECTOR 0x08
#define KERNEL_DATA_SELECTOR 0x10

// interrupts
#define PEACHOS_TOTAL_INTERRUPTS 512

// memory
#define PEACHOS_HEAP_SIZE_BYTES 104857600 // 100 MB
#define PEACHOS_HEAP_BLOCK_SIZE 4096
#define PEACHOS_HEAP_ADDRESS 0x01000000 // https://wiki.osdev.org/Memory_Map_(x86)
#define PEACHOS_HEAP_TABLE_ADDRESS 0x00007E00 // ends @ 0x0007FFFF (480 KB usable memory segment)

// disk
#define PEACHOS_SECTOR_SIZE 512

// filesystem
#define PEACHOS_MAX_FILESYSTEMS 12
#define PEACHOS_MAX_FILE_DESCRIPTORS 512
#define PEACHOS_MAX_PATH 108

// global descriptor table (GDT)
#define PEACHOS_TOTAL_GDT_SEGMENTS 6 // null segment, kernel code segment, kernel data segment, user code segment, user data segment, tss segment

// tasks
#define PEACHOS_PROGRAM_VIRTUAL_ADDRESS 0x400000
#define PEACHOS_USER_PROGRAM_STACK_SIZE (1024 * 16) // 16 KB stack
#define PEACHOS_PROGRAM_VIRTUAL_STACK_ADDRESS_START 0x3FF000
#define PEACHOS_PROGRAM_VIRTUAL_STACK_ADDRESS_END (PEACHOS_PROGRAM_VIRTUAL_STACK_ADDRESS_START - PEACHOS_USER_PROGRAM_STACK_SIZE) // stack grows downwards on intel chips
#define USER_DATA_SEGMENT 0x23 // GDT offset
#define USER_CODE_SEGMENT 0x1B // ...

// process
#define PEACHOS_MAX_PROGRAM_ALLOCATIONS 1024
#define PEACHOS_MAX_PROCESSES 12
#define PEACH_MAX_ISR80H_COMMANDS 1024 // kernel calls
