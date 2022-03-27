#pragma once
#include <stdint.h>
#include <stddef.h>

void kheap_init();
void* kmalloc( size_t size );
void* kzalloc( size_t size );
void kfree( void* p );
void* kheap_clone( void* buffer, size_t size );
