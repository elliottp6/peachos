; assembly section
section .asm

; imports
extern int21h_handler
extern no_interrupt_handler
extern isr80h_handler

; exports
global enable_interrupts
global disable_interrupts
global idt_load
global int21h
global no_interrupt
global isr80h_wrapper

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

int21h: ; note: interrupts will be auto-disabled upon entry b/c of type of interrupt (idt_desc type_attr), so we don't need cli/sti to enable/disable interrupts
    pushad ; push all general purpose registers
    call int21h_handler
    popad ; pop all general purpose registers
    iret ; return from interrupt

no_interrupt:
    pushad ; push all general purpose registers
    call no_interrupt_handler
    popad ; pop all general purpose registers
    iret ; return from interrupt
 
isr80h_wrapper:
    ; interrupt frame start
    pushad ; push all general purpose registers (uint32_t ip, cs, flags, sp, ss already pushed by processor before entering this handler)
    ; interrupt frame end

    ; call isr80h_handler
    push esp ; push arg: stack pointer (points to interrupt frame)
    push eax ; push arg: contains the command that our kernel will invoke
    call isr80h_handler ; call handler
    mov dword[return_code], eax ; stash return code (which is in eax)
    add esp, 8 ; pop arguments

    ; restore general purpose regs from interrupt frame start
    popad
    mov eax, [return_code]
    iretd

; data section
section .data

; temporary space to hold return code for isr80h_handler
return_code: dd 0
