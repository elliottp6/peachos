// https://www.udemy.com/course/developing-a-multithreaded-kernel-from-scratch/learn/lecture/23983244
// https://wiki.osdev.org/ATA_read/write_sectors
#include "disk.h"
#include "io/io.h"
#include "config.h"
#include "status.h"
#include "memory/memory.h"
#include "kernel.h"

struct disk disk; // primary hard disk

int disk_read_sector( int lba, int total_blocks, void* buffer ) { // lba = logical block address
    // seek
    outb( 0x1F6, (lba >> 24) | 0xE0 );
    outb( 0x1F2, total_blocks );
    outb( 0x1F3, (uint8_t)(lba & 0xFF) );
    outb( 0x1F4, (uint8_t)(lba >> 8) );
    outb( 0x1F5, (uint8_t)(lba >> 16) );
    outb( 0x1F7, 0x20 );

    // read
    uint16_t *p = (uint16_t*)buffer;
    for( int b = 0; b < total_blocks; b++ ) {
        // wait for the buffer to be ready (read from port until bit 0x8 is set)
        char c;
        do c = insb( 0x1F7 ); while( !(c & 0x08 ) );

        // copy 512-byte sector from hard disk to memory in 2-byte increments
        for( int i = 0; i < 256; i++, *p = insw( 0x1F0 ), p++ );
    }
    return 0;
}

int disk_search_and_init() {
    memset( &disk, 0, sizeof( disk ) );
    disk.type = PEACHOS_DISK_TYPE_REAL;
    disk.sector_size = PEACHOS_SECTOR_SIZE;
    disk.id = 0;
    disk.filesystem = fs_resolve( &disk );
    if( NULL == disk.filesystem ) return -EIO;
    return 0;
}

struct disk* disk_get( int index ) {
    if( 0 != index ) return NULL;
    return &disk;
}

int disk_read_block( struct disk* idisk, uint32_t lba, int total, void* buffer ) {
    if( idisk != &disk ) {
        if( NULL == idisk ) print( "FAILED: disk_read_block: idisk is NULL\n" );
        print( "FAILED: disk_read_block: idisk != &disk\n" );
        return -EIO;
    }
    return disk_read_sector( lba, total, buffer );
}
