	/*
	 * Create and align an initial stack
	 */
	.data
	.p2align 12
.globl	base_stack_start
base_stack_start:	
	.space	32768

.globl	base_stack_end
base_stack_end:

.globl	magic_value
magic_value:
	.space	4

.globl	boot_info_pa
boot_info_pa:
	.space	4
