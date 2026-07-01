.set MAGIC, 0xe85250d6
.set ARCH, 0
.set HEADER_LEN, multiboot2_header_end - multiboot2_header_start
.set CHECKSUM, 0x100000000 - (MAGIC + ARCH + HEADER_LEN)

.section .multiboot
.align 8
multiboot2_header_start:
    .long MAGIC
    .long ARCH
    .long HEADER_LEN
    .long CHECKSUM
    .word 0
    .word 0
    .long 8
multiboot2_header_end:


.section .text
.extern kernelMain
.extern callConstructors
.global loader


loader:
    mov $kernel_stack, %esp
    call callConstructors
    push %eax
    push %ebx
    call kernelMain


_stop:
    cli
    hlt
    jmp _stop


.section .bss
.space 2 * 1024 * 1024; # 2 MiB
kernel_stack:
