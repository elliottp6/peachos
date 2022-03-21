section .asm ; when linking, put this in the unaligned ASM section

global int21h
global idt_load ; export this symbol
global no_interrupt
global enable_interrupts
global disable_interrupts

extern int21h_handler ; import this symbol
extern no_interrupt_handler

enable_interrupts:
    sti
    ret

disable_interrupts:
    cli
    ret

idt_load:
    ; C function entry
    push ebp
    mov ebp, esp

    ; function body
    mov ebx, [ebp+8] ; ebp = base pointer, ebp + 4 = return address, rbp+8 = 1st arg
    lidt [ebx] ; load interrupt descriptor table from address 'ebx', which is 1st argument to idt_load

    ; C function exit
    pop ebp
    ret

int21h:
    cli ; disable interrupts
    pushad ; push all general purpose registers
    call int21h_handler
    popad ; pop all general purpose registers
    sti ; enable interrupts
    iret ; return from interrupt

no_interrupt:
    cli ; disable interrupts
    pushad ; push all general purpose registers
    call no_interrupt_handler
    popad ; pop all general purpose registers
    sti ; enable interrupts
    iret ; return from interrupt
 