#pragma once
#include <stddef.h>
#include <stdbool.h>

void print( const char* message );
void* peachos_malloc( size_t size );
void peachos_free( void* ptr );
void peachos_putchar( char c );
int peachos_getkey();
int peachos_getkey_block();
void peachos_terminal_readline( char* out, int max, bool output_while_typing );
void peachos_process_load_start( const char* filename );
