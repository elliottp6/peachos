[BITS 32]

global print:function
global getkey:function

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

; int getkey();
getkey:
    ; create stack frame
    push ebp
    mov ebp, esp

    ; body
    mov eax, 2 ; command 'getkey'
    int 0x80 ; note that eax contains return value, and in C the convention is that eax contains return value if it can fit into 4 bytes

    ; destroy stack frame
    pop ebp
    ret
