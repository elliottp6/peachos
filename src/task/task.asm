[BITS 32]
section .asm

global restore_general_purpose_registers
global task_return
global user_registers

; this drops us into userland
; note that the values pushed are in virtual address space, b/c we have paging enabled
; void task_return( struct registers* regs );
task_return:
    ; stack pointer => base pointer (note that we didn't save the callers base pointer, why?)
    mov ebp, esp

    ; push the task's data/stack selector & stack pointer
    mov ebx, [ebp+4]
    push dword [ebx+44] ; push push registers->ss (stack segment, data/stack selector)
    push dword [ebx+40] ; push push registers->esp (stack pointer)

    ; push flags
    pushf ; push flags register onto stack
    pop eax ; pop flags into eax
    or eax, 0x200 ; add flag to enable interrupts after the next iret instruction
    push eax ; push it back

    push dword [ebx+32] ; push registers->cs (code segment)
    push dword [ebx+28] ; push registers->ip (instruction pointer)

    ; Setup some segment registers
    mov ax, [ebx+44]
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; call 'restore general_purpose_registers'
    push dword [ebp+4] ; push a copy of the pointer to 1st arg so it becomes 1st arg for 'restore_general_purpose_registers'
    call restore_general_purpose_registers
    add esp, 4 ; need to pop stack, but cannot b/c it would corrupt the registers that we just wrote! So, do the pop like this instead.

    ; Let's leave kernel land and execute in user land!
    iretd

; this function takes a registers struct, and puts values into real registers (thus, overwriting them)
; void restore_general_purpose_registers( struct registers* regs );
; TODO: no reason to push/pop the caller's base pointer since we don't use the stack inside here, right?
restore_general_purpose_registers:
    ; C function entry
    push ebp ; save caller's base pointer
    mov ebp, esp ; current stack pointer is new base pointer, pointing to saved EBP value

    mov ebx, [ebp+8] ; move 1st argument into ebx (note that ebp+4 contains return address)
    mov edi, [ebx] ; regs[0] aka regs->edi
    mov esi, [ebx+4] ; regs[1] aka regs->esi
    mov ebp, [ebx+8] ; ...
    mov edx, [ebx+16]
    mov ecx, [ebx+20]
    mov eax, [ebx+24]
    mov ebx, [ebx+12] ; do ebx last, since that held our pointer to the argument

    ; C function exit
    pop ebp
    ret

; change all the segment registers to the USER_DATA_SEGMENT
; void user_registers();
user_registers:
    mov ax, 0x23
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    ret
