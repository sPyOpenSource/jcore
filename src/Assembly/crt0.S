.section .multiboot_header
.align  8
boot_hdr:
	.long 0xe85250d6                             /* magic number */
    .long 0x0000000                              /* architecture (0 = i386) */
	.long boot_entry - boot_hdr         
    .long 0x100000000 - (0xe85250d6 + 0x00000000 + (boot_entry - boot_hdr)) 
    .align 8
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
boot_entry:
.section .text
.globl     _start
_start:
	movl	%eax, magic_value
	movl	%ebx, boot_info_pa
	movl	$base_stack_end, %esp
	pushl	$0
	popf
	lea		edata, %edi
	lea		end, %ecx
	subl	%edi,%ecx
	xorl	%eax,%eax
	rep
	stosb


	#xorb	%al,%al
	#movw	$0x3f2,%dx
	#outb	%al,%dx
	#movw	$0x372,%dx
	#outb	%al,%dx

	pushl	boot_info_pa
    pushl   magic_value
	xorl	%ebp,%ebp
    call	 multiboot_main  
    movl %ebx, %esi
    addl $8, %esi                              # Sla de eerste 8 bytes (mbi_size + gereserveerd) over. Hier starten de tags.

find_framebuffer_tag:
    movl (%esi), %eax                          # EAX = Tag Type
    movl 4(%esi), %ecx                         # ECX = Tag Size

    cmpl $0, %eax                              # Is het de End Tag (Type 0)?
    je 1                                       # Framebuffer niet gevonden, stop hier.

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

1:      
        int   $3
        jmp   2f
        .ascii "PANIC: main returned!"
2:      
        jmp   1b
