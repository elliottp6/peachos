#pragma once
#include <stdbool.h>

int strlen( const char* p );
int strnlen( const char* p, int max );
char* strcpy( char* dest, const char* src );
char* strncpy( char* dest, const char* src, int count );
int strncmp( const char* s1, const char* s2, int len );
int istrncmp( const char* s1, const char* s2, int len );
int strnlen_terminator( const char* s, int max, char terminator );
bool is_digit( char c );
int to_numeric_digit( char c );
char tolower( char c );
void string_append( char** out, const char* in, char terminator, int max_length );
