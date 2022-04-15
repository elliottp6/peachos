#pragma once

// PS/2 port controller I/O constants (https://wiki.osdev.org/%228042%22_PS/2_Controller)
#define PS2_PORT 0x64 // PS/2 command register
#define KEYBOARD_INPUT_PORT 0x60 // PS/2 data input port
#define PS2_COMMAND_ENABLE_FIRST_PORT 0xAE //
#define CLASSIC_KEYBOARD_KEY_RELEASED 0x80 // bitmask for a single bit of the scancode
#define ISR_KEYBOARD_INTERRUPT 0x21 // interrupt called when key pressed/released

// functions
struct keyboard* classic_init();
