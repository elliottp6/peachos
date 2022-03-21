; PROBLEM: need to do something on boot
; SOLUTION: 1st sector is loaded into memory @ 0x7C00, so execute this code to print 'A'
; (note that we're in 16-bit real mode, so we have a segmented memory model)
ORG 0x7C00 ; code origin
BITS 16 ; 16-bit code

start:
    mov ah, 0eh ; command VIDEO TELETYPE OUTPUT (display char on screen & advance cursor)
    mov al, 'A' ; arg 1 (character for display)
    mov bx, 0 ; arg 2 (page number)
    int 0x10 ; calls BIOS interrupt (lookup Ralf Brown's interrupt list)
    jmp $ ; jump to self ($ = self) which is an infinite loop

; padding & signature
times 510-($ - $$) db 0 ; fill 510 - (size = (location - origin)) to bring us to 510 bytes
dw 0xAA55 ; signature (two bytes), bringing us to 512 bytes in total (sizeof boot sector)
