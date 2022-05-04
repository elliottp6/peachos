[BITS 32]

section .asm ; safer to use this to avoid alignment issues that would happen if we mixed this with C object code

global print:function
global peachos_getkey:function
global peachos_malloc:function
global peachos_free:function
global peachos_putchar:function
global peachos_process_load_start:function
global peachos_process_get_arguments:function
global peachos_system:function
global peachos_exit:function

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

; int peachos_getkey();
peachos_getkey:
    ; create stack frame
    push ebp
    mov ebp, esp

    ; body
    mov eax, 2 ; command 'getkey'
    int 0x80 ; note that eax contains return value, and in C the convention is that eax contains return value if it can fit into 4 bytes

    ; destroy stack frame
    pop ebp
    ret

; void peachos_putchar( char c );
peachos_putchar:
    ; create stack frame
    push ebp
    mov ebp, esp

    ; body
    mov eax, 3 ; command 'putchar'
    push dword[ebp + 8] ; arg #1
    int 0x80 ; syscall
    add esp, 4 ; pop args

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

; void peachos_free( void* ptr );
peachos_free:
    ; create stack frame
    push ebp
    mov ebp, esp
    
    ; body
    mov eax, 5 ; command 'free'
    push dword[ebp + 8]; push arg #1
    int 0x80
    add esp, 4 ; pop args

    ; destroy stack frame
    pop ebp
    ret

peachos_process_load_start:
    ; create stack frame
    push ebp
    mov ebp, esp
    
    ; body
    mov eax, 6 ; command 'process_load_start'
    push dword[ebp + 8]; push arg #1 'filename'
    int 0x80
    add esp, 4 ; pop args

    ; destroy stack frame
    pop ebp
    ret

; int peachos_system( struct command_argument* arguments );
peachos_system:
    ; create stack frame
    push ebp
    mov ebp, esp

    ; body
    mov eax, 7 ; command 'peachos_system'
    push dword[ebp + 8]; push arg #1 'arguments'
    int 0x80
    add esp, 4 ; pop args

    ; destroy stack frame
    pop ebp
    ret

; void peachos_process_get_arguments( struct process_arguments* args );
peachos_process_get_arguments:
    ; create stack frame
    push ebp
    mov ebp, esp
    
    ; body
    mov eax, 8 ; command 'get_program_arguments'
    push dword[ebp + 8]; push arg #1 'process_arguments*'
    int 0x80
    add esp, 4 ; pop args

    ; destroy stack frame
    pop ebp
    ret

; void peachos_exit();
peachos_exit:
    ; create stack frame
    push ebp
    mov ebp, esp
    
    ; body
    mov eax, 9 ; command 'exit'
    int 0x80

    ; destroy stack frame
    pop ebp
    ret
