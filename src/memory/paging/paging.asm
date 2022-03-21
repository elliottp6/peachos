[BITS 32]

section .asm

global paging_load_directory
global enable_paging

paging_load_directory:
    ; C function entry (save base pointer & establish new base pointer)
    push ebp
    mov ebp, esp

    ; set CR3 register to 1st argument value (which should be directory address)
    mov eax, [ebp+8]
    mov cr3, eax

    ; C function exit (restore base pointer)
    pop ebp
    ret

enable_paging:
    ; save base pointer
    push ebp
    mov ebp, esp

    ; turn on high bit in CR0
    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax

    ; restore base pointer
    pop ebp
    ret
