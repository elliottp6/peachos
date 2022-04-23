#pragma once
#include <stddef.h>

void print( const char* message );
int peachos_getkey();
int peachos_getkey_block();
void* peachos_malloc( size_t size );
void peachos_free( void* ptr );
void peachos_putchar( char c );
