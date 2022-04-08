#include "idt.h"
#include "config.h"
#include "kernel.h"
#include "memory/memory.h"
#include "io/io.h"

// interrupt descriptor table
struct idt_desc idt_descriptors[PEACHOS_TOTAL_INTERRUPTS];
struct idtr_desc idtr_descriptor;

// defined in idt.asm
extern void idt_load( struct idtr_desc* p );
extern void int21h();
extern void no_interrupt();

void idt_set( int i, void* func ) {
    struct idt_desc* desc = &idt_descriptors[i];
    desc->offset_1 = (uint32_t)func & 0xFFFF;
    desc->selector = KERNEL_CODE_SELECTOR;
    desc->zero = 0;
    desc->type_attr = 0xEE; // bits: Present = 1, DPL (descriptor privilege level) = 11 (ring 3), storage segment = 0 (must be 0 for interrupt & trap gates), gate type = 11 (3 = interrupt gate), reserved = 0
    desc->offset_2 = (uint32_t)func >> 16;
}

void idt_zero() {
    // do something to handle the interrupt
    print( "divide by zero error\n" );
}

void int21h_handler() {
    // do something to handle the interrupt
    // TODO: we must actually read the key in order to recieve more keyboard interrupts
    print( "keyboard pressed!\n" );

    // must send an acknowledgement that the interrupt was handled (or else, processor will stop sending interrupts)
    outb( 0x20, 0x20 );
}

void no_interrupt_handler() {
    // must send an acknowledgement that the interrupt was handled (or else, processor will stop sending interrupts)
    outb( 0x20, 0x20 );
}

void idt_init() {
    // clear descriptors & setup IDTR
    memset( idt_descriptors, 0, sizeof( idt_descriptors ) );
    idtr_descriptor.limit = sizeof( idt_descriptors ) - 1;
    idtr_descriptor.base = (uint32_t)idt_descriptors;

    // set each interrupt to 'no_interrupt' function
    // you MUST do this, or else the CPU will reset itself after trying to call a null interrupt
    for( int i = 0; i < PEACHOS_TOTAL_INTERRUPTS; i++ ) idt_set( i, no_interrupt );

    // set interrupt 0 (divide by zero exception)
    idt_set( 0, idt_zero );

    // set interrupt 0x21 (keyboard input)
    idt_set( 0x21, int21h );

    // load the interrupt descriptor table
    idt_load( &idtr_descriptor );
}

void isr80h_handle_command( int command, struct interrupt_frame* frame ) {
    // TODO
}

void* isr80h_handler( int command, struct interrupt_frame* frame ) {
    void* res = 0;

    // switch to the kernel page
    kernel_page();

    // copy interrupt_frame's registers into task (this allows us to switch tasks if we wanted to)
    task_current_save_state( frame );

    // run the command
    res = isr80h_handle_command( command, frame );

    // switch back to task page
    task_page();
    return res;
}
