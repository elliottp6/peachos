[BITS 32]

section .asm

global _start

_start:
    mov eax, 0 ; tells kernel to run command #0 (which is our 'sum' command)
    push 20 ; 1st argument
    push 30 ; 2nd argument
    int 0x80 ; call interrupt 0x80
    add esp, 8 ; remove arguments from stack
    jmp $
