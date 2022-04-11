// interrupt service routines for interrupt 0x80
#pragma once

enum SystemCommand {
    SYSTEM_COMMAND0_SUM,
    SYSTEM_COMMAND1_PRINT,
};

void isr80h_register_commands();
