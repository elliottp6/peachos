#include "stdlib.h"
#include "peachos.h"

// converts an integer to a string, and places it into static memory
char* itoa( int i ) {
    // static memory to hold output
    static char text[12];
    
    // put in the null terminator
    text[11] = 0;

    // ?
    char neg = 1;
    if( i >= 0 ) { neg = 0; i = -i; }

    // ?
    int loc = 11;
    for(; i; i/= 10 ) text[--loc] = '0' - (i % 10);

    // ?
    if( 11 == loc ) text[--loc] = '0';

    // add sign
    if( neg ) text[--loc] = '-';

    // return reference to static buffer
    return &text[loc];
}

void* malloc( size_t size ) {
    return peachos_malloc( size );
}

void free( void* ptr ) {
    peachos_free( ptr );
}
