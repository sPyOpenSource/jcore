MEMORY
{
    ram (rwx) : ORIGIN = 0x80200000, LENGTH = 0x03E00000
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
        __bss_start = .;
        *(.bss*)
        *(COMMON)
        __bss_end = .;
    } > ram

    stack_top_svc = ORIGIN(ram) + LENGTH(ram);
    stack_top_irq = stack_top_svc - 0x1000;
    stack_top_fiq = stack_top_irq - 0x1000;
}
