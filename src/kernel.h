#pragma once

#define VGA_WIDTH 80
#define VGA_HEIGHT 20
#define ERROR(value) (void*)(value)
#define ERROR_I(value) (int)(value)
#define ISERR(value) ((int)(value) < 0)

void kernel_main();
void kernel_page(); // switch to kernel segment and page directory
void kernel_registers();
void print( const char* str );
void panic( const char* msg );
