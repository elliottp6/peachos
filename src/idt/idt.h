// https://www.udemy.com/course/developing-a-multithreaded-kernel-from-scratch/learn/lecture/23972666
#pragma once
#include <stdint.h>

// function signature for 0x80 interrupt (kernel call from userland)
struct interrupt_frame;
typedef void*(*ISR80H_COMMAND)( struct interrupt_frame* frame );

// function signature for C function which in invoked to handle interrupts
typedef void(*INTERRUPT_CALLBACK_FUNCTION)();

// interrupt descriptor
struct idt_desc {
    uint16_t offset_1; // offset bits 0 - 15
    uint16_t selector; // code segment selector in GDT (or LDT)
    uint8_t zero; // does nothing
    uint8_t type_attr; // descriptor type & attribute flags
    uint16_t offset_2; // offset bits 16 - 31
} __attribute__((packed));


// interrupt table descriptor (pointed to by the IDT register, loaded via LIDT instruction)
struct idtr_desc {
    uint16_t limit; // sizeof descriptor table - 1 (aka size)
    uint32_t base; // base address for descriptor table (aka offset)
} __attribute__((packed));

// holds process state so we can return from an interrupted process
struct interrupt_frame {
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t reserved;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
    uint32_t ip;
    uint32_t cs;
    uint32_t flags;
    uint32_t esp;
    uint32_t ss;
} __attribute__((packed));

// initialize the interrupt descriptor table
void idt_init();
int idt_register_interrupt_callback( int interrupt, INTERRUPT_CALLBACK_FUNCTION callback );

// enable/disable interrupts
void enable_interrupts();
void disable_interrupts();

// register kernel commands that can be called from userspace
void isr80h_register_command( int command, ISR80H_COMMAND function );
