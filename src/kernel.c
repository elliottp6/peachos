#include "kernel.h"
#include <stddef.h>
#include <stdint.h>
#include "idt/idt.h"
#include "io/io.h"
#include "memory/heap/kheap.h"
#include "memory/paging/paging.h"
#include "fs/file.h"
#include "disk/disk.h"
#include "string/string.h"
#include "fs/pparser.h"
#include "disk/streamer.h"
#include "string/string.h"

uint16_t* video_mem = 0, terminal_row = 0, terminal_col = 0;

uint16_t terminal_make_char( char c, char color ) { return (color << 8) | c; }

void terminal_putchar( int x, int y, char c, char color ) {
    video_mem[(y * VGA_WIDTH) + x] = terminal_make_char( c, color );
}

void terminal_writechar( char c, char color ) {
    if( '\n' == c ) { terminal_row++; terminal_col = 0; return; }
    terminal_putchar( terminal_col, terminal_row, c, color );
    terminal_col++;
    if( terminal_col >= VGA_WIDTH ) { terminal_col = 0; terminal_row++; }
}

void print( const char* str ) {
    size_t len = strlen( str );
    for( int i = 0; i < len; i++ ) terminal_writechar( str[i], 15 );
}

void terminal_initialize() {
    video_mem = (uint16_t*)0xB8000;
    for( int y = 0; y < VGA_HEIGHT; y++ )
        for( int x = 0; x < VGA_WIDTH; x++ )
            terminal_putchar( x, y, ' ', 0 );
}

extern void problem();

static struct paging_4gb_chunk* kernel_chunk = 0; 

void kernel_main() {
    // initialize terminal
    terminal_initialize();
    print( "initialized terminal\n" );

    // initialize the kernel heap
    kheap_init();
    print( "initialized kernel heap\n" );

    // initialize filesystems
    fs_init();
    print( "initialized filesystems\n" );

    // search & initialize the disks
    disk_search_and_init();
    print( "found ATA disk @ index 0\n" );

    // initialize interrupt descriptor table
    idt_init();
    print( "initialized interrupt descriptor table\n" );

    // initialize page tables
    kernel_chunk = paging_new_4gb( PAGING_IS_WRITABLE | PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL );
    paging_switch( paging_4gb_chunk_get_directory( kernel_chunk ) );
    enable_paging();
    print( "initialized page tables\n" );

    //  --test page tables -- : map virtual address '0x1000' to 'p'
    //char *p = kzalloc( 4096 ), *p2 = (char*)0x1000;
    //paging_set( paging_4gb_chunk_get_directory( kernel_chunk ), (void*)0x1000, (uint32_t)p | PAGING_ACCESS_FROM_ALL | PAGING_IS_PRESENT | PAGING_IS_WRITABLE );
    //p2[0] = 'A';
    //p2[1] = 'B';
    //print( p2 ); // p2 => p
    //print( p ); // p => p

    // enable interrupts
    enable_interrupts();
    print( "enabled interrupts\n" );

    // -- test strcpy --
    //char buf[20];
    //strcpy( buf, "hello!" );
    //print( buf );
    //while( 1 ) {}

    // -- test disk streamer --
    // first, read first sector
    struct disk* disk = disk_get( 0 );
    uint8_t buffer[512];
    disk_read_block( disk, 0, 1, buffer );
    if( 235 == buffer[0] ) print( "TEST: DISK BLOCK PASSED\n" ); else print( "TEST: DISK BLOCK FAILED\n" );

    // next, read using the stream
    struct disk_stream* stream = diskstreamer_new( 0 );
    diskstreamer_seek( stream, 0x201 );
    uint8_t c = 0;
    diskstreamer_read( stream, &c, 1 );
    if( 184 == c ) print( "TEST: STREAM READ PASSED\n" ); else print( "TEST: STREAM READ FAILED\n" );

    // -- test path parser --
    //struct path_root* root_path = pathparser_parse( "0:/bin/shell.exe", NULL );
    //if( root_path ) {
    //    if( 0 == root_path->drive_no ) print( "0:/" );
    //    print( root_path->first->part );
    //    print( "/" );
    //    print( root_path->first->next->part );
    //}

    // -- test to see if the disk's filesystem resolved --
    if( NULL != disk->filesystem ) {
        print( "Filesystem bound: '" );
        print( disk->filesystem->name );
        print( "'\n" );
    } else print( "Failed to resolve disk 0 filesystem!\n" );

    //  --test the kernel heap --
    //void* p1 = kmalloc( 50 );
    //void* p2 = kmalloc( 5000 );
    //void* p3 = kmalloc( 5600 );
    //kfree( p1 );
    //void* p4 = kmalloc( 50 );
    //if( p1 || p2 || p3 || p4 ) {}
    
    //  --write 0xff to port 0x60 --
    // outb( 0x60, 0xff );

    // wait forever
    while( 1 );
}