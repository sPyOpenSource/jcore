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
.extern long_mode_start
.global loader

.code32
loader:
    mov $pdpt_table, %eax
    or $3, %eax
    mov %eax, pml4_table

    mov $pdpt_table, %eax
    or $3, %eax
    mov %eax, pdpt_table

    mov $0, %ecx
.loop_pd:
    mov $0x200000, %eax
    mul %ecx
    or $0x83, %eax
    mov %eax, pd_table(,%ecx,8)
    inc %ecx
    cmp $512, %ecx
    jne .loop_pd

    mov %cr4, %eax
    or $(1 << 5), %eax
    mov %eax, %cr4

    mov $pml4_table, %eax
    mov %eax, %cr3

    mov $0xC0000080, %ecx
    rdmsr
    or $(1 << 8), %eax
    wrmsr

    mov %cr0, %eax
    or $(1 << 31), %eax
    mov %eax, %cr0

    lgdt gdt64_ptr
    ljmp $0x08, $long_mode_start

.align 4096
pml4_table:
    .skip 4096
pdpt_table:
    .skip 4096
pd_table:
    .skip 4096

.align 8
gdt64:
    .quad 0
    .quad (1 << 43) | (1 << 44) | (1 << 47) | (1 << 53)
    .quad (1 << 44) | (1 << 47) | (1 << 41)
gdt64_ptr:
    .word gdt64_ptr - gdt64 - 1
    .long gdt64
    .long 0
