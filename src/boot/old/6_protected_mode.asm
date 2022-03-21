; https://www.udemy.com/course/developing-a-multithreaded-kernel-from-scratch/learn/lecture/23972348
ORG 0x7C00 ; code origin
BITS 16 ; 16-bit code

; defines
CODE_SEG equ gdt_code - gdt_start ; offset to gdt_code (should be 0x08)
DATA_SEG equ gdt_data - gdt_start ; offset to gdt_data (should be 0x10)

; BPB (BIOS parameter block)
BPB:
    jmp short start ; relative jump to 'start'
    nop
times 33 db 0

; jump to main16, specifying the code segment as 0
start:
    jmp 0:main16

; main16 (real mode)
main16:
    ; set real mode segment registers
    cli ; disable interrupts
    mov ax, 0
    mov ds, ax ; data segment = 0
    mov es, ax ; extra segment = 0
    mov ss, ax ; stack segment = 0
    mov sp, 0x7C00 ; stack pointer = 0x7C00 (stack grows down)
    sti ; enable interrupts
load_protected:
    ; enter protected mode
    cli ; disable interrupts
    lgdt[gdt_descriptor] ; load GDT
    mov eax, cr0 ; turn on 1st bit of cr0
    or eax, 1
    mov cr0, eax
    jmp CODE_SEG:main32

; GDT (global descriptor table)
gdt_start:
gdt_null:
    dd 0x0
    dd 0x0
gdt_code: ; offset 0x8, linked to CS
    dw 0xffff
    dw 0 ; base first 0-15 bits
    db 0 ; base 16-32 bits
    db 0x9A ; access byte
    db 11001111b ; high 4 bit flats & low 4 bit flags
    db 0 ; base 24-31 bits
gdt_data: ; offset 0x10, linked to DS, SS, ES, FS, GS
    dw 0xffff
    dw 0 ; base first 0-15 bits
    db 0 ; base 16-32 bits
    db 0x92 ; access byte
    db 11001111b ; high 4 bit flats & low 4 bit flags
    db 0 ; base 24-31 bits
gdt_end:
gdt_descriptor:
    dw gdt_end - gdt_start - 1 ; size
    dd gdt_start ; offset

; main32 (protected mode)
[BITS 32]
main32:
    ; setup protected mode segment registers
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov ebp, 0x00200000
    mov esp, ebp
    jmp $

; padding & signature
times 510-($ - $$) db 0 ; fill 510 - (size = (location - origin)) to bring us to 510 bytes
dw 0xAA55 ; signature (two bytes), bringing us to 512 bytes in total (sizeof boot sector)
