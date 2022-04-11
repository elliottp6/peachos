[BITS 32]

section .asm

global _start

_start:
    ; call kernel add command
    ;mov eax, 0 ; tells kernel to run command #0 (which is our 'sum' command)
    ;push 20 ; 1st argument
    ;push 30 ; 2nd argument
    ;int 0x80 ; call interrupt 0x80
    ;add esp, 8 ; pop arguments
    
    ; call kernel print command
    mov eax, 1 ; command #1
    push message ; 1st argument
    int 0x80
    add esp, 4 ; pop arguments

    ; infinite loop
    jmp $

section .data
message: db 'Hello world from user process!', 10, 0
