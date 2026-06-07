.section .multiboot_header
.align  8  
boot_hdr:
	.long 0xe85250d6                             /* magic number */
    .long 0                                      /* architecture (0 = i386) */
	.long (boot_entry - boot_hdr)
    .long 0x100000000 - (0xe85250d6 + 0 + (boot_entry - boot_hdr))   
    .word 0                         
	.word 0                         
    .long 8                                   /* header length */
boot_entry:
.section .text
.p2align  2   , 0x90 ; .type       _start   ,@function ; .globl     _start     ;     _start:    
	movl	$base_stack_end, %esp
	pushl	$0
	popf
	lea	edata, %edi
	lea	end, %ecx
	subl	%edi,%ecx
	xorl	%eax,%eax
	rep
	stosb

	 
	xorb	%al,%al
	movw	$0x3f2,%dx		 
	outb	%al,%dx
	movw	$0x372,%dx		 
	outb	%al,%dx


	pushl	%ebx

	xorl	%ebp,%ebp
	 
	call	 multiboot_main  

1:      
        int   $3
        jmp   2f
        .ascii "PANIC: main returned!"
2:      
        jmp   1b
