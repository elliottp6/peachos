; https://c9x.me/x86/html/file_module_x86_id_139.html

section .asm

; exported functions
global insb
global insw
global outb
global outw

insb:
    ; C function entry
    push ebp
    mov ebp, esp

    ; function body
    xor eax, eax ; zero out the eax register
    mov edx, [ebp+8] ; grab 1st argument, put into edx
    in al, dx ; read from port (lower 16 bits of edx) into lower 8 bits of eax
    
    ; C function exit (note that eax is always the return value, by convention)
    pop ebp
    ret

insw:
    ; C function entry
    push ebp
    mov ebp, esp

    ; function body
    xor eax, eax ; zero out the eax register
    mov edx, [ebp+8] ; grab 1st argument, put into edx
    in ax, dx ; read from port (lower 16 bits of edx) into ax
    
    ; C function exit (note that eax is always the return value, by convention)
    pop ebp
    ret

outb:
    ; C function entry
    push ebp
    mov ebp, esp

    ; function body
    mov eax, [ebp+12] ; grab 2nd argument (value)
    mov edx, [ebp+8] ; grab 1st argument (port)
    out dx, al ; write al to port dx

    ; C function exit
    pop ebp
    ret

outw:
    ; C function entry
    push ebp
    mov ebp, esp

    ; function body
    mov eax, [ebp+12] ; grab 2nd argument (value)
    mov edx, [ebp+8] ; grab 1st argument (port)
    out dx, ax ; write ax to port dx

    ; C function exit (note that eax is always the return value, by convention)
    pop ebp
    ret
