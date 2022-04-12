#include "classic.h"
#include "keyboard.h"
#include "io/io.h"
#include <stdint.h>
#include <stddef.h>

// forward delcaration
int classic_keyboard_init();

// lookup table: scancode index => ascii value (https://wiki.osdev.org/PS/2_Keyboard, section "Scan Code Set 1")
static uint8_t keyboard_scan_set_one[] = {
    0x00, 0x1B, '1', '2', '3', '4', '5',
    '6', '7', '8', '9', '0', '-', '=',
    0x08, '\t', 'Q', 'W', 'E', 'R', 'T',
    'Y', 'U', 'I', 'O', 'P', '[', ']',
    0x0d, 0x00, 'A', 'S', 'D', 'F', 'G',
    'H', 'J', 'K', 'L', ';', '\'', '`', 
    0x00, '\\', 'Z', 'X', 'C', 'V', 'B',
    'N', 'M', ',', '.', '/', 0x00, '*',
    0x00, 0x20, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, '7', '8', '9', '-', '4', '5',
    '6', '+', '1', '2', '3', '0', '.'
};

// keyboard object
struct keyboard classic_keyboard = {
    .name = { "Classic" },
    .init = classic_keyboard_init
};

// functions
int classic_keyboard_init() {
    // PS/2 port controller
    // PS2_PORT (0x64) is the "command register" for the PS/2 port controller
    // PS2_COMMAND_ENABLE_FIRST_PORT (0xAE) command Enables the first PS/2 port
    outb( PS2_PORT, PS2_COMMAND_ENABLE_FIRST_PORT );

    // done
    return 0;
}

uint8_t classic_keyboard_scancode_to_char( uint8_t scancode ) {
    // bounds check TODO: bugfix in lecture, should be >= rather than >
    size_t size_of_keyboard_set_one = sizeof( keyboard_scan_set_one ) / sizeof( uint8_t );
    if( scancode >= size_of_keyboard_set_one ) return 0;

    // decode the character
    return keyboard_scan_set_one[scancode];
}

void classic_keyboard_handle_interrupt() {
    // TODO
}

struct keyboard* classic_init() { return &classic_keyboard; }
