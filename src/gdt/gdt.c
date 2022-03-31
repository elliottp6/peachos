#include "gdt.h"
#include "kernel.h"

static void encode_gdt_entry( uint8_t* target, struct gdt_structured source ) {
    // encode the limit
    if( (source.limit > 65536) && (0xFFF != (0xFFF & source.limit)) ) panic( "encode_gdt_entry: invalid argument\n" );
    if( source.limit > 65536 ) { source.limit>>=12; target[6] = 0xC0; } else target[6] = 0x40;
    target[0] = source.limit & 0xFF;
    target[1] = (source.limit >> 8) & 0xFF;
    target[6]|= (source.limit >> 16) & 0xFF;
    
    // encode the base
    target[2] = source.base & 0xFF;
    target[3] = (source.base >> 8) & 0xFF;
    target[4]|= (source.base >> 16) & 0xFF;
    target[7] = (source.base >> 24) & 0xFF; // TODO: & 0xFF isn't needed here

    // encode the type
    target[5] = source.type;
}

void gdt_structured_to_gdt( struct gdt* gdt, struct gdt_structured* structured_gdt, int total_entries ) {
    for( int i = 0; i < total_entries; i++ ) encode_gdt_entry( (uint8_t*)&gdt[i], structured_gdt[i] );
}
