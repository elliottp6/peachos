ENTRY(_start) /* begin execution @ _start, which is in the bootloader */
OUTPUT_FORMAT(elf32-i386) /* ELF file format */
SECTIONS
{
    . = 0x400000; /* program virtual origin = PEACHOS_PROGRAM_VIRTUAL_ADDRESS 0x400000 */

    .text : ALIGN(4096) /* kernel.asm (which is internally aligned to 16-bytes), then 16-byte aligned C code goes here */
    {
        *(.text)
    }

    .asm : ALIGN(4096) /* unaligned assembly code goes here */
    {
        *(.asm)
    }

    .rodata : ALIGN(4096) /* readonly data section */
    {
        *(.rodata)
    }

    .data : ALIGN(4096) /* data section */
    {
        *(.data)
    }

    .bss : ALIGN(4096) /* static data section */
    {
        *(COMMON)
        *(.bss)
    }
}
