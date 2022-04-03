#include "string.h"
#include <stdint.h>
#include <stddef.h>

char tolower( char c ) { return c >= 65 && c <= 90 ? c + 32 : c; }

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

int strnlen_terminator( const char* s, int max, char terminator ) {
    int i;
    for( i = 0; i < max && s[i] && s[i] != terminator; i++ );
    return i;
}

int istrncmp( const char* s1, const char* s2, int len ) { // case-insensitive version of strncmp
    uint8_t *p1 = (uint8_t*)s1, *p2 = (uint8_t*)s2;
    for( int i = 0; i < len; i++ ) {
        if( tolower( p1[i] ) != tolower( p2[i] ) ) return (int)p1[i] - (int)p2[i];
        if( !p1[i] ) break;
    }
    return 0;
}

int strncmp( const char* s1, const char* s2, int len ) {
    uint8_t *p1 = (uint8_t*)s1, *p2 = (uint8_t*)s2;
    for( int i = 0; i < len; i++ ) {
        if( p1[i] != p2[i] ) return (int)p1[i] - (int)p2[i];
        if( !p1[i] ) break;
    }
    return 0;
}

char* strcpy( char* dest, const char* src ) {
    int i = 0;
    do dest[i] = src[i]; while( src[i++] ); // do-while loop ensures we copy the null terminator!
    return dest;
}

char* strncpy( char* dest, const char* src, int count ) {
    int i = 0;
    for( i = 0; i < count - 1 && src[i]; i++ ) dest[i] = src[i];
    dest[i] = NULL;
    return dest;
}

bool is_digit( char c ) { return c >= 48 && c <= 57; }
int to_numeric_digit( char c ) { return c - 48; }

void string_replace_terminator_with_null_terminator( char** out, const char* in, char terminator ) {
    while( *in && *in != terminator ) { **out = *in; *out+=1; in+=1; }
    if( *in == terminator ) **out = 0;
}
