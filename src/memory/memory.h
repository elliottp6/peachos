#pragma once
#include <stddef.h>
#include <stdint.h>

void* memset( void* buffer, uint8_t value, size_t size );
int memcmp( void* s1, void* s2, int count );
void* memcpy( void* dest, void* src, int len );
