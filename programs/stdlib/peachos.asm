[BITS 32]

global print:function

; void print( const char* message );
print:
    ; create stack frame
    push ebp
    mov ebp, esp

    ; body
    push dword[ebp+8] ; argument #1
    mov eax, 1 ; command 'print'
    int 0x80 ; syscall
    add esp, 4 ; pop arguments

    ; destroy stack frame
    pop ebp
    ret
