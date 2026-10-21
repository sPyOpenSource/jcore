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
	# TAG 1: Framebuffer Request (ask GRUB for a graphics mode)
    .align 8
    .word 5                                    # Type 5 = Framebuffer request
    .word 0                                    # Flags
    .long 20                                   # Size of this tag (20 bytes)
    .long 1024                                 # Width (X)
    .long 768                                  # Height (Y)
    .long 32                                   # Bits per pixel (BPP)

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

1:
    cli
    hlt
    jmp   1b
    .ascii "PANIC: main returned!"
