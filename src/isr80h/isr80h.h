// interrupt service routines for interrupt 0x80
#pragma once

enum SystemCommand {
    SYSTEM_COMMAND0_SUM
};

void isr80h_register_commands();
