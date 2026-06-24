// AArch32 mode - fixed version (no .org 0x8000, linker script handles it)
.section ".text.boot"
.globl _start
_start:
	// Shut off extra cores
	mrc p15, 0, r5, c0, c0, 5
	and r5, r5, #3
	cmp r5, #0
	bne halt

	// Setup the stack.
	ldr r5, =_start
	mov sp, r5

	// Clear out bss.
	ldr r4, =__bss_start
	ldr r9, =__bss_end
	mov r5, #0
	mov r6, #0
	mov r7, #0
	mov r8, #0
	b       2f
1:
	stmia r4!, {r5-r8}
2:
	cmp r4, r9
	blo 1b

	// Call main
	ldr r3, =main
	blx r3

halt:
	wfe
	b halt
