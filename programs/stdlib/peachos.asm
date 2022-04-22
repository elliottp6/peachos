[BITS 32]

global print:function
global getkey:function
global peachos_malloc:function

; void print( const char* message );
print:
    ; create stack frame
    push ebp
    mov ebp, esp

    ; body
    mov eax, 1 ; command 'print'
    push dword[ebp + 8] ; arg #1
    int 0x80 ; syscall
    add esp, 4 ; pop args

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

; void* peachos_malloc( size_t size );
peachos_malloc:
    ; create stack frame
    push ebp
    mov ebp, esp
    
    ; body
    mov eax, 4 ; command 'malloc'
    push dword[ebp + 8]; push arg #1
    int 0x80
    add esp, 4 ; pop args

    ; destroy stack frame
    pop ebp
    ret
