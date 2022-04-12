#include "idt.h"
#include "config.h"
#include "kernel.h"
#include "memory/memory.h"
#include "task/task.h"
#include "io/io.h"

// interrupt descriptor table
struct idt_desc idt_descriptors[PEACHOS_TOTAL_INTERRUPTS];
struct idtr_desc idtr_descriptor;

// pointers to assembly routines which handle interrupts
extern void* interrupt_pointer_table[PEACHOS_TOTAL_INTERRUPTS];

// function pointers for 0x80 interrupt (kernel call from userland)
static ISR80H_COMMAND isr80h_commands[PEACH_MAX_ISR80H_COMMANDS];

// defined in idt.asm
extern void idt_load( struct idtr_desc* p );
extern void int21h();
extern void no_interrupt();
extern void isr80h_wrapper();

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

void no_interrupt_handler() {
    // must send an acknowledgement that the interrupt was handled (or else, processor will stop sending interrupts)
    outb( 0x20, 0x20 );
}

void interrupt_handler( int interrupt, struct interrupt_frame* frame ) {
    // TODO: handle interrupts

    // send acknowledgement that interrupt was handled
    outb( 0x20, 0x20 );
}

void idt_init() {
    // clear descriptors & setup IDTR
    memset( idt_descriptors, 0, sizeof( idt_descriptors ) );
    idtr_descriptor.limit = sizeof( idt_descriptors ) - 1;
    idtr_descriptor.base = (uint32_t)idt_descriptors;

    // set each interrupt to point to the assembly routine we defined for it
    for( int i = 0; i < PEACHOS_TOTAL_INTERRUPTS; i++ )
        idt_set( i, interrupt_pointer_table[i] );

    // set interrupt 0 (divide by zero exception)
    idt_set( 0, idt_zero );

    // kernel call from userland
    idt_set( 0x80, isr80h_wrapper );

    // load the interrupt descriptor table
    idt_load( &idtr_descriptor );
}

void isr80h_register_command( int command, ISR80H_COMMAND function ) {
    // bounds check
    if( command < 0 || command >= PEACH_MAX_ISR80H_COMMANDS ) panic( "attempt to register a kernel command that is out-of-bounds\n" );

    // empty slot check
    if( isr80h_commands[command] )  panic( "attempted to overwrite an existing command\n" );

    // set slot
    isr80h_commands[command] = function;
}

void* isr80h_handle_command( int command, struct interrupt_frame* frame ) {
    // bounds check
    if( command < 0 || command >= PEACH_MAX_ISR80H_COMMANDS ) return NULL;

    // get function pointer
    ISR80H_COMMAND function = isr80h_commands[command];
    if( !function ) return NULL;

    // execute function & return result
    return function( frame );
}

void* isr80h_handler( int command, struct interrupt_frame* frame ) {
    // switch to the kernel page
    kernel_page();

    // copy interrupt_frame's registers into task (this allows us to switch tasks if we wanted to)
    task_current_save_state( frame );

    // get the command
    void* res = isr80h_handle_command( command, frame );

    // switch back to task page
    task_page();
    return res;
}
