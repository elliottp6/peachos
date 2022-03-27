#include "pparser.h"
#include "kernel.h"
#include "string/string.h"
#include "memory/heap/kheap.h"
#include "memory/memory.h"
#include "status.h"
#include "config.h"

// 0:/folder/file.ext
static int pathparser_path_valid_format( const char* filename ) {
    int len = strnlen( filename, PEACHOS_MAX_PATH );
    return len >= 3 && is_digit( filename[0] ) && 0 == memcmp( (void*)&filename[1], ":/", 2 );
}

// WARNING: this also mutates the path pointer argument by moving it forward by 3 bytes
static int pathparser_get_drive_by_path( const char** path ) {
    // validate path format
    if( !pathparser_path_valid_format( *path ) ) return -EBADPATH;

    // get drive number
    int drive_no = to_numeric_digit( *path[0] );

    // add 3 to skip drive number
    *path += 3;
    return drive_no;
}

static struct path_root* pathparser_create_root( int drive_number ) {
    struct path_root* path_r = kzalloc( sizeof( struct path_root ) );
    path_r->drive_no = drive_number;
    path_r->first = 0;
    return path_r;
}

// WARNING: this mutates the argument by moving it forward to next '/'
// BUG: if memory allocation fails, this function will try to access null pointer
static const char* pathparser_get_path_part( const char** path ) {
    // allocate string
    char* result_path_part = kzalloc( PEACHOS_MAX_PATH );

    // copy string
    const char* read = *path;
    int i = 0;
    for( i = 0; read[i] && '/' != read[i]; i++ ) result_path_part[i] = read[i];

    // skip last '/'
    if( '/' == read[i] ) i++;

    // if nothing parsed, free pointer
    if( 0 == i ) { kfree( result_path_part ); result_path_part = NULL; }

    // eat characters in 'path' before returning result
    *path+=i;
    return result_path_part;
}

struct path_part* pathparser_parse_path_part( struct path_part* last_part, const char** path ) {
    // get the next path part string
    const char* path_part_str = pathparser_get_path_part( path );
    if( !path_part_str ) return NULL;

    // create path_part object
    struct path_part* part = kzalloc( sizeof( struct path_part ) );
    part->part = path_part_str;
    part->next = NULL;

    // point the tail to this
    if( last_part ) last_part->next = part;
    return part;
}

void pathparser_free( struct path_root* root ) {
    struct path_part* part = root->first;
    while( part ) {
        struct path_part* next_part = part->next;
        kfree( (void*)part->part );
        kfree( (void*)part );
        part = next_part;
    }
    kfree( root );
}

struct path_root* pathparser_parse( const char* path, const char* current_directory_path ) {
    int res = 0;
    const char* temp_path = path;
    struct path_root* path_root = 0;

    // check path length
    if( strlen( path ) > PEACHOS_MAX_PATH ) goto out;

    // get drive number
    res = pathparser_get_drive_by_path( &temp_path );
    if( res < 0 ) goto out;

    // get root part
    path_root = pathparser_create_root( res );
    if( !path_root ) goto out;

    // get 1st path part
    struct path_part* first_part = pathparser_parse_path_part( NULL, &temp_path );
    if( !first_part ) goto out;
    path_root->first = first_part;

    // get rest path parts
    struct path_part* part = first_part;
    do part = pathparser_parse_path_part( part, &temp_path ); while( part );

out:
    return path_root;
}
