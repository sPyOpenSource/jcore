# Teaching workspace initialised for RPi boot chain

Established the teaching workspace in EFI_AARCH64 project root. User wants to build a bare-metal OS for Raspberry Pi, grounded in their existing work with AARCH64 UEFI on QEMU.

## Prior knowledge (disclosed)
- Has a working AARCH64 UEFI boot loader (`loader/bootboot.c`) and kernel running on QEMU
- Has an AArch32 RPi 2 boot stub in `rpi2/` (boot.S, linker.ld, main.c with UART/GPIO)
- Comfortable with cross-compilation for AARCH64 using CLANG
- Understands ARMv8 exception levels and basic MMIO
- The UEFI world has "spoiled" them — UEFI provides rich boot services. Bare-metal RPi does not. This will be a recurring theme

## Next session recommendation
Lesson 2: "Your First AArch64 Kernel on Real Hardware" — build the minimal boot.S + linker.ld + C entry that runs on a real Pi 3/4 via SD card, with UART output.
