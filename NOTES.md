# Teaching Notes — RPi Boot Chain

- User has existing EFI_AARCH64 project running on QEMU
- Already has rpi2/ with 32-bit AArch32 boot stub and linker script
- Primary interest: understanding the boot chain so they can run their kernel on real hardware
- Cross-compiles with CLANG on macOS
- Goal is "build OS from scratch" — wants deep understanding, not just copy-paste
