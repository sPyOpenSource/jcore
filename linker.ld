MEMORY
{
    ram (rwx) : ORIGIN = 0x80200000, LENGTH = 0x10000
}

ENTRY(_start)

SECTIONS
{
    .text : {
        *(.text*)
    } > ram

    .rodata : {
        *(.rodata*)
    } > ram

    .data : {
        *(.data*)
    } > ram

    .bss : {
        *(.bss*)
    } > ram
}
