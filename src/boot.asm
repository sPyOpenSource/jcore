# Enable FPU/NEON, set stack, jump to main()
# Works for both BBAI and QEMU (stack address from linker)

.arm
.global _start

_start:
    # Enable access to CP10 (VFP) and CP11 (NEON) in CPACR
    mrc p15, 0, r0, c1, c0, 2
    orr r0, r0, #(0xF << 20)
    mcr p15, 0, r0, c1, c0, 2
    isb

    # Set FPEXC enable bit to turn on the FPU/NEON unit
    mov r0, #0x40000000
    vmsr fpexc, r0
    isb

    # Stack pointer from linker script (_stack_top = ORIGIN + LENGTH)
    ldr sp, =_stack_top

    bl main

_hang:
    b _hang
