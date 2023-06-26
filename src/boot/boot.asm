; https://www.udemy.com/course/developing-a-multithreaded-kernel-from-scratch/learn/lecture/23972412

; constants
ORG 0x7C00 ; code origin (since this is where bios will load 1st sector (512 bytes) into memory)
BITS 16 ; 16-bit real mode (segmented memory model)
CODE_SEG equ gdt_code - gdt_start ; offset to gdt_code (should be 0x08)
DATA_SEG equ gdt_data - gdt_start ; offset to gdt_data (should be 0x10)

; --BPB (BIOS parameter block) --
; This describes the physical layout of a data storage volume.
; Filesystems which use a BPB include FAT12/16/32, HPFS, NTFS.
_start:
    jmp short start ; relative jump to 'start', which skips over the BPB, and starts executing code
    nop
OEMIdentifier       db 'PEACHOS '; OEM Identifier for PEACHOS (FAT16 header) https://www.udemy.com/course/developing-a-multithreaded-kernel-from-scratch/learn/lecture/23984420, https://en.wikipedia.org/wiki/Design_of_the_FAT_file_system
BytesPerSector      dw 0x200 ; 512 bytes per sector (generally ignored by most kernels)
SectorsPerCluster   db 0x80 ; 128 sectors per cluster (i.e. 65K per cluster)
ReservedSectors     dw 200 ; plenty for our entire kernel (we won't be loading the kernel from file, but from reserved sectors)
FATCopies           db 0x02 ; original & backup
RootDirEntries      dw 0x40 ; # of entries in the root directory
NumSectors          dw 0x00
MediaType           db 0xF8
SectorsPerFat       dw 0x100 ; 256
SectorsPerTrack     dw 0x20
NumberOfHeads       dw 0x40
HiddenSectors       dd 0x00
SectorsBig          dd 0x773594
DriveNumber         db 0x80 ; Extended BPB (Dos 4.0)
WinNTBit            db 0x00
Signature           db 0x29
VolumeID            dd 0xD105
VolumeIDString      db 'PEACHOS BOO' ; must be 11 bytes
SystemIDString      db 'FAT16   ' ; must be 8 bytes

; -- execution continues from here --
; jump to main16 (in real mode, we use segment:offset for addresses (where address = segment * 16 + offset)
; (not sure why we do this rather than jumping directly to main16 from _start label, this seems like an extra unneeded jump)
start:
    jmp 0:main16

; -- execution continues from here --
; main16 (real mode)
main16:
    ; clear real mode segment registers
    cli ; disable interrupts
    mov ax, 0
    mov ds, ax ; data segment = 0
    mov es, ax ; extra segment = 0
    mov ss, ax ; stack segment = 0
    mov sp, 0x7C00 ; stack pointer = 0x7C00 (stack grows down)
    sti ; enable interrupts
load_protected:
    ; enter protected mode (32-bit)
    cli ; disable interrupts
    lgdt[gdt_descriptor] ; load GDT
    mov eax, cr0 ; turn on 1st bit of cr0
    or eax, 1
    mov cr0, eax
    jmp CODE_SEG:load32
    
; GDT (global descriptor table)
gdt_start:
gdt_null: ; null segment
    dd 0x0
    dd 0x0
gdt_code: ; code segment, offset 0x8, linked to CS
    dw 0xffff
    dw 0 ; base first 0-15 bits
    db 0 ; base 16-32 bits
    db 0x9A ; access byte
    db 11001111b ; high 4 bit flats & low 4 bit flags
    db 0 ; base 24-31 bits
gdt_data: ; data segment, offset 0x10, linked to DS, SS, ES, FS, GS
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

; loads the kernel into memory
[BITS 32]
load32:
    mov eax, 1 ; LBA 1 (logical block address 1, which is just past the bootloader)
    mov ecx, 100 ; 100 sectors (entire kernel)
    mov edi, 0x100000 ; loads kernel into 1MB position in memory
    call ata_lba_read ; loads kernel into memory
    jmp CODE_SEG:0x100000 ; jump to kernel '_start'

ata_lba_read:
    ; send the highest 8 bits of the LBA to the HD controller
    mov ebx, eax ; backup LBA (logical block address)
    shr eax, 24
    or eax, 0xE0 ; select the master drive
    mov dx, 0x1F6 ; this is the port that use to talk to the HD controller
    out dx, al ; set port
    
    ; send the total sectors to read to port 0x1F2
    mov eax, ecx 
    mov dx, 0x1F2
    out dx, al

    ; send more bits of the LBA
    mov eax, ebx ; restoring LBA to EAX
    mov dx, 0x1F3
    out dx, al

    ; send more bits of the LBA
    mov dx, 0x1F4
    mov eax, ebx; restore the backup LBA
    shr eax, 8
    out dx, al

    ; send upper 16 bits of the LBA
    mov dx, 0x1F5
    mov eax, ebx ; restor the backup LBA
    shr eax, 16
    out dx, al

    ; ???
    mov dx, 0x1F7
    mov al, 0x20
    out dx, al

    ; read all sectors into memory
.next_sector:
    push ecx

.try_again:
    ; check if we need to read
    mov dx, 0x1F7
    in al, dx
    test al, 8
    jz .try_again

    ; need to read 256 words at a time
    mov ecx, 256
    mov dx, 0x1F0
    rep insw ; rep = repeat 'ecx' times: insw = read a word from port 'dx' and store into where 'edi' points to???
    pop ecx ; restore ecx (total # of sectors to read)
    loop .next_sector ; loop while decrementing ecx

    ; done
    ret

; padding & signature
times 510-($ - $$) db 0
dw 0xAA55 ; signature (two bytes), bringing us to 512 bytes in total (sizeof boot sector)
