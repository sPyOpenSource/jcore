# Tested 32-bit RPi 2 kernel in QEMU

Successfully cross-compiled and booted `kernel.c` as a 32-bit RPi 2 kernel on QEMU's `raspi2b` machine, with UART output ("Hello, kernel World!") visible on `-serial stdio`.

## Key practical findings

- **`qemu -kernel` is unreliable for raw RPi binaries** — use `-device loader,file=...,addr=0x8000,force-raw=on` instead. QEMU's `-kernel` path expects a Linux boot protocol header, not a raw binary.
- **`.org` in boot.S must not double-offset.** When the linker script sets `. = 0x8000`, the `.org 0x8000` pushed code to 0x10000. Fixed version (`boot_fixed.S`) removes `.org`.
- **Toolchain mix works:** CLANG (`--target=arm-none-eabi`) compiles 32-bit ARM; the AArch64 GNU `ld -marmelf` links it. No separate 32-bit ARM toolchain needed on macOS.
- **UART init on RPi 2 needs PL011 at 0x3F201000** (MMIO base 0x3F000000 + GPIO offset 0x200000 + UART offset 0x1000).

## Build command (from rpi2/)

```
make -C /Users/xuyi/Source/OS/EFI_AARCH64/rpi2 qemu
```

## Implications
- The existing `rpi2/linker.ld` works as-is for 32-bit.
- `kernel.c` has a proper `main()` with UART, mailbox-based clock setup (for RPi 3+), and echo loop. It was never compiled into the previous `kernel7.img`.
