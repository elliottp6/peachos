#pragma once
#include <stdbool.h>

int strlen( const char* p );
int strnlen( const char* p, int max );
char* strcpy( char* dest, const char* src );
bool is_digit( char c );
int to_numeric_digit( char c );
