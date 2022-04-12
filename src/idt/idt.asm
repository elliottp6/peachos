; assembly section
section .asm

; imports
extern int21h_handler
extern no_interrupt_handler
extern isr80h_handler
extern interrupt_handler

; exports
global enable_interrupts
global disable_interrupts
global idt_load
global no_interrupt
global isr80h_wrapper
global interrupt_pointer_table

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

no_interrupt:
    pushad ; push all general purpose registers
    call no_interrupt_handler
    popad ; pop all general purpose registers
    iret ; return from interrupt

; macro for what an interrupt handler should look like
%macro interrupt 1
    global int%1 ; export this as int0, int1, int2, ...
    int%1: ; here is the label
        ; interrupt frame start
        pushad ; push general purpose registers (ip, cs, falgs, sp, ss already pushed by cpu before calling interrupt)
        ; interrupt frame end

        ; call interrupt_handler
        push esp ; push stack pointer
        push dword %1 ; push arg1 for C function interrupt_handler
        call interrupt_handler
        add esp, 8 ; pop arguments

        ; pop general purpose registers & return
        popad
        iret
%endmacro

; create 512 instances of our interrupt
%assign i 0
%rep 512
    interrupt i
%assign i i+1
%endrep

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

; interrupt pointer table entry
%macro interrupt_array_entry 1
    dd int%1 ; label for int* function that we defined earlier
%endmacro

; create the interrupt pointer table
interrupt_pointer_table:
%assign i 0
%rep 512
    interrupt_array_entry i
%assign i i+1
%endrep
