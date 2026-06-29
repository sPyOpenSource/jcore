# BeagleBone AI (AM5729) Baremetal Boot Stub
# Entry point for the kernel loaded by U-Boot at 0x80200000

# UART base locations from the TRM
.equ UART1.BASE,    0x4806A000
.equ UART2.BASE,    0x4806C000
.equ UART3.BASE,    0x49020000
.equ UART4.BASE,    0x49042000

.arm
.global _start

_start:
    # Set up the stack pointer at the end of our RAM region.
    # RAM starts at 0x80200000, length 0x10000.
    ldr sp, =0x80210000

    # Call the C kernel entry point
    bl main

    # If main returns, hang forever
_hang:
    b _hang
