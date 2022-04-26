#include "memory.h"

void* memset( void* buffer, uint8_t value, size_t size ) {
    uint8_t* p = (uint8_t*)buffer;
    for( int i = 0; i < size; i++ ) p[i] = value;
    return buffer;
}

int memcmp( void* s1, void* s2, int count ) {
    uint8_t *b1 = (uint8_t*)s1, *b2 = (uint8_t*)s2;
    for( int i = 0; i < count; i++ ) if( b1[i] != b2[i] ) return b1[i] < b2[i] ? -1 : 1;
    return 0;
}

void* memcpy( void* dest, void* src, int len ) {
    char *d = dest, *s = src;
    while( len-- ) *d++ = *s++;
    return dest;
}
