#include "kernel.h"
#include <stddef.h>
#include <stdint.h>
#include "idt/idt.h"
#include "io/io.h"
#include "memory/memory.h"
#include "memory/heap/kheap.h"
#include "memory/paging/paging.h"
#include "disk/disk.h"
#include "string/string.h"
#include "fs/pparser.h"
#include "disk/streamer.h"
#include "string/string.h"
#include "memory/memory.h"
#include "fs/file.h"
#include "gdt/gdt.h"
#include "config.h"
#include "task/tss.h"
#include "task/task.h"
#include "task/process.h"
#include "status.h"
#include "isr80h/isr80h.h"
#include "keyboard/keyboard.h"

uint16_t* video_mem = 0, terminal_row = 0, terminal_col = 0;

uint16_t terminal_make_char( char c, char color ) { return (color << 8) | c; }

void terminal_putchar( int x, int y, char c, char color ) {
    video_mem[(y * VGA_WIDTH) + x] = terminal_make_char( c, color );
}

// TODO: this was fixed up from lecture 90, whose implementation did not handle backspaces properly
void terminal_backspace() {
    if( 0 == terminal_col ) {
        if( 0 == terminal_row ) return;
        terminal_row--;
        terminal_col = VGA_WIDTH;
    }
    terminal_putchar( --terminal_col, terminal_row, ' ', 15 );
}

void terminal_writechar( char c, char color ) {
    // handle newline 
    if( '\n' == c ) { terminal_row++; terminal_col = 0; return; }

    // handle backspace
    if( 8 == c ) { terminal_backspace(); return; }

    // handle regular character
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

void kernel_page() {
    kernel_registers();
    paging_switch( kernel_chunk );
}

void panic( const char* msg ) { print( msg ); while( 1 ); }

struct tss tss;
struct gdt gdt_real[PEACHOS_TOTAL_GDT_SEGMENTS];
struct gdt_structured gdt_structured[PEACHOS_TOTAL_GDT_SEGMENTS] = {
    {.base = 0x00, .limit = 0x00, .type = 0x00}, // null segment
    {.base = 0x00, .limit = 0xFFFFFFFF, .type = 0x9A}, // kernel code segment
    {.base = 0x00, .limit = 0xFFFFFFFF, .type = 0x92}, // kernel data segment
    {.base = 0x00, .limit = 0xFFFFFFFF, .type = 0xF8},  // user code segment (it's ok to use the same segment, b/c we're using paging to separate it from the kernel)
    {.base = 0x00, .limit = 0xFFFFFFFF, .type = 0xF2}, // user data segment
    {.base = (uint32_t)&tss, .limit = sizeof( tss ), .type = 0xE9} // tss segment
};

void pic_timer_callback() { print( "tick\n" ); }

void kernel_main() {
    // initialize terminal
    terminal_initialize();
    print( "initialized terminal\n" );

    // load the GDT
    memset( gdt_real, 0, sizeof( gdt_real ) );
    gdt_structured_to_gdt( gdt_real, gdt_structured, PEACHOS_TOTAL_GDT_SEGMENTS );
    gdt_load( gdt_real, sizeof( gdt_real ) );
    print( "loaded the GDT\n" );

    // initialize the kernel heap
    kheap_init();
    print( "initialized kernel heap\n" );

    // initialize filesystems
    fs_init();
    print( "initialized filesystems\n" );

    // test reading sector 0
    uint8_t buffer[512];
    disk_read_sector( 0, 1, buffer );
    if( 235 == buffer[0] ) print( "TEST: DISK SECTOR 0 PASSED\n" ); else print( "TEST: DISK SECTOR 0 FAILED\n" );

    // search & initialize the disks
    if( 0 == disk_search_and_init() ) print( "found FAT16 filesystem on disk @ index 0\n" );
    else print( "failed to bind disk to filesystem\n" );

    // initialize IDT (interrupt descriptor table)
    idt_init();
    print( "initialized IDT (interrupt descriptor table)\n" );

    // setup the TSS (task state segment)
    memset( &tss, 0, sizeof( tss ) );
    tss.esp0 = 0x600000;
    tss.ss0 = KERNEL_DATA_SELECTOR;
    tss_load( 0x28 );
    print( "loaded the TSS (task state segment)\n" );

    // enable paging
    kernel_chunk = paging_new_4gb( PAGING_IS_WRITEABLE | PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL );
    paging_switch( kernel_chunk );
    enable_paging();
    print( "initialized page tables & enabled paging\n" );

    // register the kernel commands
    isr80h_register_commands();
    print( "registered kernel commands\n" );

    // initialize the keyboards
    keyboard_init();
    print( "initialized keyboards\n" );

    // test: register some interrupt callbacks
    // note: disabled for now: idt_register_interrupt_callback( 0x20, pic_timer_callback ); // timer interrupt

    // load program
    struct process* process = NULL;
    int res = process_load_and_give_focus( "0:/blankc.elf", &process );
    if( PEACHOS_ALL_OK != res ) panic( "failed to load blankc.elf\n" ); else print( "Loaded blankc.elf OK\n" );

    // test: push character to current process' keyboard buffer
    // (note: we cannot pop, because there's no task to pop yet)
    // keyboard_push( 'A' );

    // run first task
    task_run_first_ever_task();

    // enable interrupts
    enable_interrupts();
    print( "enabled interrupts\n" );

    // test opening a file
    /*
    int fd = fopen( "0:/hello.txt", "r" );
    if( !fd ) print( "could NOT open hello.txt\n" );
    else {
        print( "opened hello.txt\n" );

        // seek & read from file
        char buf[14];
        memset( buf, 0, sizeof( buf ) );
        fseek( fd, 2, SEEK_SET );
        fread( buf, 11, 1, fd );
        print( buf );
        print( "\n" );

        // get file status
        struct file_stat stat;
        fstat( fd, &stat );
        if( 12 == stat.filesize ) print( "got 12 bytes for size\n" ); else print( "size was incorrect\n" );

        // close the file
        if( 0 == fclose( fd ) ) print( "closed file successfully\n" );
    }
    while( 1 );
    */

    //  --test page tables -- : map virtual address '0x1000' to 'p'
    //char *p = kzalloc( 4096 ), *p2 = (char*)0x1000;
    //paging_set( paging_4gb_chunk_get_directory( kernel_chunk ), (void*)0x1000, (uint32_t)p | PAGING_ACCESS_FROM_ALL | PAGING_IS_PRESENT | PAGING_IS_WRITABLE );
    //p2[0] = 'A';
    //p2[1] = 'B';
    //print( p2 ); // p2 => p
    //print( p ); // p => p

    // -- test strcpy --
    //char buf[20];
    //strcpy( buf, "hello!" );
    //print( buf );
    //while( 1 ) {}

    // -- test disk streamer --

    // next, read using the stream
    //struct disk_stream* stream = diskstreamer_new( 0 );
    //diskstreamer_seek( stream, 0x201 );
    //uint8_t c = 0;
    //diskstreamer_read( stream, &c, 1 );
    //if( 184 == c ) print( "TEST: STREAM READ PASSED\n" ); else print( "TEST: STREAM READ FAILED\n" );

    // -- test path parser --
    //struct path_root* root_path = pathparser_parse( "0:/bin/shell.exe", NULL );
    //if( root_path ) {
    //    if( 0 == root_path->drive_no ) print( "0:/" );
    //    print( root_path->first->part );
    //    print( "/" );
    //    print( root_path->first->next->part );
    //}

    // -- test to see if the disk's filesystem resolved --
    //struct disk* disk = disk_get( 0 );
    //if( NULL != disk->filesystem ) {
    //    print( "Filesystem bound: '" );
    //    print( disk->filesystem->name );
    //    print( "'\n" );
    //} else print( "Failed to resolve disk 0 filesystem!\n" );

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
    //while( 1 );
}
