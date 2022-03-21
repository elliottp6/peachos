#pragma once
#include <stdbool.h>

int strlen( const char* p );
int strnlen( const char* p, int max );
bool is_digit( char c );
int to_numeric_digit( char c );
