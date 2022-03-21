// https://www.udemy.com/course/developing-a-multithreaded-kernel-from-scratch/learn/lecture/23972688
#pragma once
#include <stdint.h>

uint8_t insb( uint16_t port );
uint16_t insw( uint16_t port );

void outb( uint16_t port, uint8_t value );
void outw( uint16_t port, uint16_t value );
