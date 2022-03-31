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
#define PEACHOS_TOTAL_GDT_SEGMENTS 3 // null segment, code segment, data segment
