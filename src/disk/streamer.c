#include "streamer.h"
#include "memory/heap/kheap.h"
#include "config.h"
#include "kernel.h"
#include <stdbool.h>

struct disk_stream* diskstreamer_new( int disk_id ) {
    // get disk
    struct disk* disk = disk_get( disk_id );
    if( !disk ) {
        print( "FAILED to get diskstreamer_new disk\n" );
        return NULL;
    }

    // allocate disk streamer
    struct disk_stream* streamer = kzalloc( sizeof( struct disk_stream ) ); // TODO: if( NULL == streamer ) return NULL;
    streamer->pos = 0;
    streamer->disk = disk;
    return streamer;
}

int diskstreamer_seek( struct disk_stream* stream, int pos ) {
    stream->pos = pos;
    return 0;
}

int diskstreamer_read( struct disk_stream* stream, void* out_buffer, int total ) {
    // allocate temporary buffer on stack
    int sector = stream->pos / PEACHOS_SECTOR_SIZE,
        offset = stream->pos % PEACHOS_SECTOR_SIZE,
        total_to_read = total;
    bool overflow = (offset + total_to_read) >= PEACHOS_SECTOR_SIZE;
    char buffer[PEACHOS_SECTOR_SIZE];

    // reduce amount read so we don't overflow the buffer
    if( overflow ) total_to_read -= (offset + total_to_read) - PEACHOS_SECTOR_SIZE;

    // read a single sector
    int res = disk_read_block( stream->disk, sector, 1, buffer );
    if( res < 0 ) { print( "FAILED to disk_read_block\n" ); return res; }

    // copy sector to out_buffer
    for( int i = 0; i < total_to_read; i++ ) *(char*)out_buffer++ = buffer[offset + i];

    // adjust the stream
    stream->pos += total_to_read;

    // read more
    if( overflow ) return diskstreamer_read( stream, out_buffer, total - total_to_read );
    return 0;
}

void diskstreamer_close( struct disk_stream* stream ) { kfree( stream ); }
