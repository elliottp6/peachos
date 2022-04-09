; code type
[BITS 32]

; imports
extern kernel_main

; exports
global _start
global kernel_registers

; defines
CODE_SEG equ 0x08
DATA_SEG equ 0x10

; protected mode
_start:
    ; setup protected mode segment registers
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov ebp, 0x00200000
    mov esp, ebp

    ; enable the A20 line for physical memory
    ; (disabled for historical 8086 has a quirk called "memory wraparound" that some programs relied on, so the 20th bit was latched to zero).
    ; we MUST enable this so we have access to all of memory
    in al, 0x92 ; read from port 0x92
    or al, 2
    out 0x92, al ; write to port 0x92

    ; Remap the master PIC (programmable interrupt controller)
    ; required b/c, by default, some IRQs are mapped to interrupts 8-15
    ; but these interrupts are reserved in protected mode for exceptions
    ; the PIC's master port handles IRQ 0-7, the PIC's slave port handles IRQ 8-15
    ; PIC control ports: 0x20 (command) and 0x21 (data) for master IRQs, 0xA0 (command) and 0xA1 (data) for slave IRQs
    mov al, 00010001b ; 4=1: init, b3=0: edge, b1=0: cascade, b0=1: need 4th init step
    out 0x20, al ; 0x20 = command
    mov al, 0x20 ; master IRQ should be on INT 0x20 (just after intel exceptions)
    out 0x21, al ; 0x21 = data
    mov al, 00000001b ; b4=0: FNM, b3-2=00: master/slave set by hardware; b1=0: not AEOI; b0=1: x86 mode
    out 0x21, al ; 0x21 = more data

    ; enter C function 'kernel_main'
    call kernel_main

    ; divide by zero (to test our custom interrupt)
    ;mov eax, 0
    ;div eax

    ; infinite loop
    jmp $

; change segment registers to point to the kernel data segment
kernel_registers:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov gs, ax
    mov fs, ax
    ret

; padding to make this file 16-byte aligned (which allows it to mix w/ C object files in the same text section)
times 512-($ - $$) db 0
