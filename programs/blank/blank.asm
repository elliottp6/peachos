[BITS 32]

section .asm

global _start

_start:
    mov eax, 0 ; tells kernel which command to run
    int 0x80 ; call interrupt 0x80
    jmp $
