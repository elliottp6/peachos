; https://www.udemy.com/course/developing-a-multithreaded-kernel-from-scratch/learn/lecture/23967250
; https://wiki.osdev.org/Exceptions
; INTERRUPT VECTOR TABLE
ORG 0 ; code origin
BITS 16 ; 16-bit code

; BIOS parameter block
BPB:
    jmp short start ; relative jump to 'start'
    nop
times 33 db 0

; jump to main, specifying the code segment as 0x7C0
start:
    jmp 0x7C0:main

; main
main:
    ; set segment registers
    cli ; disable interrupts
    mov ax, 0x7C0
    mov ds, ax ; data segment = 0x7C0
    mov es, ax ; extra segment = 0x7C0
    mov ax, 0x00
    mov ss, ax ; stack segment = 0
    mov sp, 0x7C00 ; stack pointer = 0x7C00 (stack grows down)
    sti ; enable interrupts

    ; build interrupt table
    mov word[ss:0x00], interrupt_0 ; interrupt[0].offset
    mov word[ss:0x02], 0x7C0 ; interrupt[0].segment
    mov word[ss:0x04], interrupt_1 ; interrupt[1].offset
    mov word[ss:0x06], 0x7C0 ; interrupt[1].segment

    ; test interrupt 0
    int 0 ; call interrupt 0
    mov ax, 0
    div ax ; cause interrupt 0 to execute via a division-by-zero

    ; test interrupt 1
    int 1

    ; write message
    mov si, message ; put message pointer into si register
    call print ; call print

    ; done (infinite loop)
    jmp $

; interrupt 0: "divide by zero"
interrupt_0:
    mov ah, 0eh
    mov al, '0'
    mov bx, 0x80
    int 0x10
    iret

; interrupt 1: "debug"
interrupt_1:
    mov ah, 0eh
    mov al, '1'
    mov bx, 0x80
    int 0x10
    iret

; function
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

; function
print_char:
    mov ah, 0eh ; command VIDEO TELETYPE OUTPUT (display char on screen & advance cursor)
    int 0x10 ; calls BIOS interrupt (lookup Ralf Brown's interrupt list)
    ret

; static data
message:
    db 'Hello world #3!', 0 ; message w/ null terminator

; padding & signature
times 510-($ - $$) db 0 ; fill 510 - (size = (location - origin)) to bring us to 510 bytes
dw 0xAA55 ; signature (two bytes), bringing us to 512 bytes in total (sizeof boot sector)
