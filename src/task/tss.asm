section .asm

global tss_load

tss_load:
    ; C function entry
    push ebp
    mov ebp, esp

    ; body
    mov ax, [ebp+8] ; get arg1 (TSS pointer) and move it into 'ax'
    ltr ax ; load task register, which points to the TSS (task state segment)
    
    ; C function exit
    pop ebp
    ret
