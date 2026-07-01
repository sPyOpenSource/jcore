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

.code32
loader:
    # 1. Setup Page Tables (Identity mapping 0..1GB)
    mov $pml4_table, %eax
    or $3, %eax               # present | writable
    mov %eax, pml4_table

    mov $pdpt_table, %eax
    or $3, %eax               # present | writable
    mov %eax, pdpt_table

    # PD[0..511] -> 2MB pages
    mov $0, %ecx              # counter
.loop_pd:
    mov $0x200000, %eax       # 2MB
    mul %ecx
    or $0x83, %eax            # present | writable | huge (bit 7)
    mov %eax, pd_table(,%ecx,8)
    inc %ecx
    cmp $512, %ecx
    jne .loop_pd

    # 2. Enable PAE
    mov %cr4, %eax
    or $(1 << 5), %eax
    mov %eax, %cr4

    # 3. Load CR3
    mov $pml4_table, %eax
    mov %eax, %cr3

    # 4. Enable Long Mode in EFER MSR
    mov $0xC0000080, %ecx
    rdmsr
    or $(1 << 8), %eax         # LME bit
    wrmsr

    # 5. Enable Paging
    mov %cr0, %eax
    or $(1 << 31), %eax        # PG bit
    mov %eax, %cr0

    # 6. Load 64-bit GDT and Far Jump
    lgdt gdt64_ptr
    ljmp $0x08, $long_mode_start

.align 16
.code64
long_mode_start:
    # Now in 64-bit mode
    mov $0, %ax
    mov %ax, %ss
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs

    # Setup 64-bit stack
    mov $kernel_stack, %rsp

    call callConstructors

    # kernelMain(multiboot_structure, multiboot_magic)
    # System V AMD64 ABI: rdi = 1st arg, rsi = 2nd arg
    # GRUB passes: eax = magic, ebx = info_struct
    movl %ebx, %edi
    movl %eax, %esi
    call kernelMain

_stop:
    cli
    hlt
    jmp _stop

.align 4096
pml4_table:
    .skip 4096
pdpt_table:
    .skip 4096
pd_table:
    .skip 4096

.align 8
gdt64:
    .quad 0                   # Null descriptor
    .quad (1 << 43) | (1 << 44) | (1 << 47) | (1 << 53) # Code segment
    .quad (1 << 44) | (1 << 47) | (1 << 41) # Data segment
gdt64_ptr:
    .word gdt64_ptr - gdt64 - 1
    .quad gdt64

.section .bss
.align 16
kernel_stack:
    .skip 2 * 1024 * 1024
