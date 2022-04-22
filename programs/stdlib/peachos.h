#pragma once
#include <stddef.h>

void print( const char* message );
int getkey();
void* peachos_malloc( size_t size );
void peachos_free( void* ptr );
void peachos_putchar( char c );
