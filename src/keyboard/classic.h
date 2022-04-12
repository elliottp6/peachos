#pragma once

// PS/2 port controller I/O constants
#define PS2_PORT 0x64
#define PS2_COMMAND_ENABLE_FIRST_PORT 0xAE

// functions
struct keyboard* classic_init();
