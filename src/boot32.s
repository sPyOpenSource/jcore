.set MAGIC, 0xe85250d6
.set ARCH, 1
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
request_start:
    .word 1
    .word 0 
    .long request_end - request_start
    .long 4
    .long 6
    .long 3
    .long 0
request_end:
	# TAG 1: Framebuffer Request (Vraag GRUB om een grafische modus)
    #.align 8
    #.word 5                                    # Type 5 = Framebuffer request
    #.word 0                                    # Flags
    #.long 20                                   # Grootte van deze tag (20 bytes)
    #.long 1024                                 # Breedte (X)
    #.long 768                                  # Hoogte (Y)
    #.long 32                                   # Bits per pixel (BPP)

    # TAG 2: End Tag
    .align 8
    .word 0                                    # Type 0 = End
    .word 0                                    # Flags
    .long 8                                    # Grootte (8 bytes)

.section .text
.extern long_mode_start
.global loader

.code32
loader:
    movl %ebx, %esi
    addl $8, %esi

find_framebuffer_tag:
    movl (%esi), %eax                          # EAX = Tag Type
    movl 4(%esi), %ecx                         # ECX = Tag Size

    cmpl $0, %eax                              # Is het de End Tag (Type 0)?
    je setup_paging                                       # Framebuffer niet gevonden, stop hier.

    cmpl $8, %eax                              # Is het de Framebuffer Tag (Type 8)?
    je framebuffer_found                       # Gevonden! Spring naar het tekenen.

    # Tag niet gevonden, ga naar de volgende tag.
    # We moeten de tag size omhoog afronden naar de dichtstbijzijnde 8-byte grens (alignment)
    addl $7, %ecx
    andl $0xFFFFFFF8, %ecx
    addl %ecx, %esi                            # ESI wijst nu naar de volgende tag
    jmp find_framebuffer_tag

framebuffer_found:
    # Volgens de Multiboot2 spec staat bij Type 8:
    # Base Addr op offset 8 (64-bit getal, we pakken de onderste 32-bit op offset 8)
    movl 8(%esi), %edi                         # EDI bevat nu het ECHTE UEFI framebuffer adres!

    # Teken een rood vlak ter controle
    movl $0x00FF0000, %eax                     # Kleur rood
    movl $40000, %ecx 
draw_loop:
    movl %eax, (%edi)                          # Schrijf pixel (32-bit)
    addl $4, %edi                              # Volgende pixel (+4 bytes voor 32-bpp)
    loop draw_loop

setup_paging:
    mov $pdpt_table, %eax
    or $3, %eax
    mov %eax, pml4_table

    mov $pd_table, %eax
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

.section .rodata
.align 8
gdt64:
    .quad 0
    .quad (1 << 43) | (1 << 44) | (1 << 47) | (1 << 53)
    .quad (1 << 44) | (1 << 47) | (1 << 41)
gdt64_ptr:
    .word gdt64_ptr - gdt64 - 1
    .long gdt64
    .long 0

.section .bss
.align 4096
pml4_table:
    .skip 4096
pdpt_table:
    .skip 4096
pd_table:
    .skip 4096