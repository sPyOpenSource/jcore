KERNEL_OFFSET = 0xfffffffff8000000;

PHDRS
{
  boot PT_LOAD FILEHDR PHDRS;                                /* one single loadable segment */
}
SECTIONS
{
    . = KERNEL_OFFSET;
    mmio    = .; . += 0x4000000;
    fb      = .; . += 0x3E00000;
    bootboot    = .; . += 4096;
    environment = .; . += 4096;

    .text . + SIZEOF_HEADERS : AT(ADDR(.text) - KERNEL_OFFSET + SIZEOF_HEADERS) {
        __text_start = .;
        KEEP(*(.text.boot)) *(.text .text.* .gnu.linkonce.t*)   /* code */
		. = ALIGN(4096);
        __text_end = .;
    } :boot

	.rodata : AT(ADDR(.rodata) - KERNEL_OFFSET) {
        __rodata_start = .;
        *(.rodata*)
		. = ALIGN(4096);
        __rodata_end = .;
    } :boot

    .data : AT(ADDR(.data) - KERNEL_OFFSET) {
        __data_start = .;
        *(.data*)
		. = ALIGN(4096);
        __data_end = .;
        __bss_start = .;
        *(.bss*)
        . = ALIGN(4096);
        __bss_end = .;
    } :boot

    /DISCARD/ : { *(.eh_frame) *(.comment) }
}
