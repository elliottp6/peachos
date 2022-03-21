; https://www.udemy.com/course/developing-a-multithreaded-kernel-from-scratch/learn/lecture/23967146
; PROBLEM: we cannot trust that the segment registers are set correctly by the BIOS
; SOLUTION: set origin to zero, then explicitly set segment registers based on 0x7C00 (where BIOS loads this sector into RAM)
; that way, all pointers in the code are 0-based, but the segment registers do the needed offsetting
ORG 0 ; code origin (where assembler assumes we are loaded into RAM)
BITS 16 ; 16-bit code

; jump to main, specifying the code segment as 0x7C0
jmp 0x7C0:main

main:
    cli ; disable interrupts
    mov ax, 0x7C0
    mov ds, ax ; data segment = 0x7C0 (i.e. memory references will be 0x7C00 + address)
    mov es, ax ; extra segment = 0x7C0
    mov ax, 0x00
    mov ss, ax ; stack segment = 0
    mov sp, 0x7C00 ; stack pointer = 0x7C00 (stack grows down)
    sti ; enable interrupts
    mov si, message ; put message pointer into si register
    call print ; call print
    jmp $ ; infinite loop

print:
    mov bx, 0 ; bx holds our counter
.loop:
    lodsb ; load si[b] into al (uses the DS:SI combo for segmented memory access)
    cmp al, 0 ; check for null terminator
    je .done ; if zero/null, jump to .done
    call print_char
    jmp .loop ; otherwise, print next char
.done:
    ret

print_char:
    mov ah, 0eh ; command VIDEO TELETYPE OUTPUT (display char on screen & advance cursor)
    int 0x10 ; calls BIOS interrupt (lookup Ralf Brown's interrupt list)
    ret

message:
    db 'Hello world #2!', 0 ; message w/ null terminator

; padding & signature
times 510-($ - $$) db 0 ; fill 510 - (size = (location - origin)) to bring us to 510 bytes
dw 0xAA55 ; signature (two bytes), bringing us to 512 bytes in total (sizeof boot sector)
