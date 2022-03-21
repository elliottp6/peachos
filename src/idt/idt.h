// https://www.udemy.com/course/developing-a-multithreaded-kernel-from-scratch/learn/lecture/23972666
#pragma once
#include <stdint.h>

// https://wiki.osdev.org/Interrupt_Descriptor_Table

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

// initialize the interrupt descriptor table
void idt_init();

// enable/disable interrupts
void enable_interrupts();
void disable_interrupts();

