	.text
.p2align  2   , 0x90 ; .type       _start   ,@function ; .globl     _start     ;     _start:
	jmp	boot_entry


	.p2align  2
boot_hdr:
	; magic number
	dd 0xe85250d6 ; multiboot2
	; architecture
	dd 0 ; protected mode i386
	; header length
	dd header_end - header_start
	; checksum
	dd 0x100000000 - (0xe85250d6 + 0 + (header_end - header_start))

	; end tag
	dw 0
	dw 0
	dd 8
boot_entry:
	movl	$base_stack_end, %esp
	pushl	$0
	popf
	lea		edata, %edi
	lea		end, %ecx
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
	lgdt[gdt64.pointer]
	jmp gdt64.code_segment:long_mode_start

1:
        int   $3
        jmp   2f
        .ascii "PANIC: main returned!"
2:
        jmp   1b
