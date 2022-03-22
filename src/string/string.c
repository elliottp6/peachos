#include "string.h"

int strlen( const char* p ) {
    int i = 0;
    while( p[i] ) i++;
    return i;
}

int strnlen( const char* p, int max ) {
    int i;
    for( i = 0; i <= max && p[i]; i++ );
    return i;
}

char* strcpy( char* dest, const char* src ) {
    int i = 0;
    do dest[i] = src[i]; while( src[i++] ); // do-while loop ensures we copy the null terminator!
    return dest;
}

bool is_digit( char c ) { return c >= 48 && c <= 57; }
int to_numeric_digit( char c ) { return c - 48; }
