[BITS 32]

section .asm

global _start

_start:
    ; print message1
    mov eax, message1
    call print_pop

    ; wait for a keypress
next_key:
    ; getkey, and goto 'done' if it's 'enter'
    call getkey
    cmp eax, 10 ; TODO: this isn't working, so we want need a different value
    je done

    ; write character
    push eax ; save the key onto the stack
    mov eax, 3 ; execute command #3 (putchar)
    int 0x80
    pop eax ; pop the key
    jmp next_key

done:
    ; print message2
    mov eax, message2
    call print_pop

    ; infinite loop
    jmp $

print_pop:
    push eax ; 1st argument is passed in eax
    mov eax, 1 ; kernel print command
    int 0x80
    add esp, 4 ; pop argument
    ret

getkey:
    mov eax, 2 ; kernel getkey command
    int 0x80
    cmp eax, 0 ; compare the return value with zero
    je getkey ; if it's zero, then do a busy wait (i.e. check again)
    ret

section .data
message1: db 'User Process: press any key to continue: ', 0
message2: db 10, 'User Process: thank you for pushing a key!', 10, 0
