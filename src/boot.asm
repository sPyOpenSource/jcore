#UART base locations from the TRM
.equ UART1.BASE, 		0x4806A000
.equ UART2.BASE, 		0x4806C000
#According to the Beagleboard-xM System Reference Manual, UART3 is connected to the serial port.
.equ UART3.BASE, 		0x49020000
.equ UART4.BASE, 		0x49042000
 
#We need to be in ARM mode - we may branch to Thumb mode later if desired.
.arm
_start:
   # Thankfully, U-BOOT already sets up serial for us, so we don't need to do all the reset crap.
   # Just print a '!' symbol to the screen, and hang.
   # Load the location of the transmit register (THR_REG) into r0. It's at the UARTx.BASE, since it is offset 0x000.
   # (load value pointed by the assembler (immediate pointer) into r0)
   ldr r0,=UART3.BASE
   # Move a '!' character into r1
   # (move a character, '!', which is immediate, into r1)
   mov r1,#'!'
   # According to the TRM, we may only write bytes into THR_REG, else we'll corrupt stuff.
   # (store the least-significant-byte of r1 into the address pointed by r0)
   strb r1,[r0]
   # If we kept writing to the serial port, we'd eventually overflow the 64-byte FIFO, and since we don't handle interrupts yet, we'll hang (?)
   # In ASM, labels are like case: statements in C. Code flows into them as if they don't exist - because they don't.
_hang:
   # (branch (jump, JMP) to _hang)
   # b _hang
