ENTRY(_start) /* begin execution @ _start, which is in the bootloader */
OUTPUT_FORMAT(binary) /* everything is binary, so not distinction between code & data (KISS) */
SECTIONS
{
    . = 1M; /* kernel origin @ 0x100000, or 1024*1024 = 1048576 */

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
