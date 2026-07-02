MEMORY
{
    ram (rwx) : ORIGIN = 0x40010000, LENGTH = 0x04000000
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

    _stack_top = ORIGIN(ram) + LENGTH(ram);
}
