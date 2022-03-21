; PROBLEM: want to write a string (not just a char)
; SOLUTION: store string in static data, define 'print' and 'print_char' functions
ORG 0x7C00 ; code origin
BITS 16 ; 16-bit code

start:
    mov si, message ; put message pointer into si register
    call print ; call print
    jmp $ ; infinite loop

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
    db 'Hello world!', 0 ; message w/ null terminator

; padding & signature
times 510-($ - $$) db 0 ; fill 510 - (size = (location - origin)) to bring us to 510 bytes
dw 0xAA55 ; signature (two bytes), bringing us to 512 bytes in total (sizeof boot sector)
