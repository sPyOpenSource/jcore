# Mission: Build a Bare-Metal OS for Raspberry Pi

## Why
You already have an AARCH64 UEFI boot loader and kernel running on QEMU. Now you want to run your own OS on real Raspberry Pi hardware — understanding the RPi boot chain (GPU-first, VideoCore firmware, config.txt, kernel loading) so you can produce a kernel image that boots on physical ARM64 hardware, not just an emulator.

## Success looks like
- Boot a self-written AArch64 kernel on a real Raspberry Pi 3 or 4 via SD card
- Understand each stage of the boot chain well enough to debug a "no boot" scenario
- Integrate UEFI-style boot concepts with RPi's proprietary GPU-first boot model
- Produce a minimal working kernel that receives DTB from the firmware and can talk to UART

## Constraints
- Hardware: Raspberry Pi 3 or 4
- Toolchain: AArch64 cross-compiler (CLANG or GCC)
- Prefers learning the boot chain depth-first: understand the stages, then write to them
- Working project context: EFI_AARCH64 UEFI loader on macOS

## Out of scope
- Full OS features (scheduling, virtual memory, filesystems) — focus on boot only for now
- Raspberry Pi 5 (BCM2712) boot chain changes — not yet covered
- VideoCore GPU programming or Videocore-side development
