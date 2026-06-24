// AArch64 boot stub for RPi 3 (kernel8.img)
// Loaded at 0x80000 by VC firmware when config.txt has arm_64bit=1
.section ".text.boot"
.globl _start
_start:
	// Park secondary cores (MPIDR_EL1 bits [1:0] = core ID)
	mrs	x5, mpidr_el1
	and	x5, x5, #3
	cbz	x5, 1f
	b	halt
1:
	// Set stack pointer just below our load address
	ldr	x5, =_start
	mov	sp, x5

	// Clear BSS
	ldr	x4, =__bss_start
	ldr	x9, =__bss_end
	b	2f
1:	stp	xzr, xzr, [x4], #16
2:	cmp	x4, x9
	blo	1b

	// Call kernel_main(dtb_ptr, x1, x2, x3)
	// RPi firmware passes DTB pointer in x0, zeroes the rest
	ldr	x3, =kernel_main
	blr	x3

halt:
	wfe
	b	halt
