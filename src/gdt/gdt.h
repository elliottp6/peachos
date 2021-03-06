#pragma once
#include <stdint.h>

struct gdt { // corresponds to the GDT that the processor requires (same as in boot.asm)
    uint16_t segment;
    uint16_t base_first;
    uint8_t base;
    uint8_t access;
    uint8_t high_flags;
    uint8_t base_24_31_bits;  
} __attribute__((packed)); // must be packed b/c the CPU directly reads this structure from memory

struct gdt_structured {
    uint32_t base;
    uint32_t limit;
    uint8_t type;
};

void gdt_load( struct gdt* gdt, int size );
void gdt_structured_to_gdt( struct gdt* gdt, struct gdt_structured* structured_gdt, int total_entries );
