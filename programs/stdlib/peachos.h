#pragma once
#include <stddef.h>
#include <stdbool.h>

// types
struct command_argument {
    char argument[512];
    struct command_argument* next;
};

// functions
void print( const char* message );
void* peachos_malloc( size_t size );
void peachos_free( void* ptr );
void peachos_putchar( char c );
int peachos_getkey();
int peachos_getkey_block();
void peachos_terminal_readline( char* out, int max, bool output_while_typing );
void peachos_process_load_start( const char* filename );
struct command_argument* peachos_parse_command( const char* command, int max );
