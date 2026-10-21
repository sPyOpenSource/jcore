# Design: Screen Output on UEFI Boot

## Problem

When JCore boots under UEFI (GRUB2 + OVMF in QEMU), the kernel's boot
messages never reach the display. The kernel assumes a legacy VGA text
buffer at `0xb8000`, which UEFI/OVMF does not provide; GRUB instead
hands over a linear framebuffer through the multiboot2 framebuffer tag
(type 8). The existing red-rectangle test in `crt0.S` was also unreachable
because it ran after `multiboot_main`, which never returns.

## Goals

- Show a visible on-screen indication that boot started when running under
  UEFI.
- Show a visible on-screen indication on fatal panic (`sys_panic`).
- Preserve the existing legacy BIOS path (VGA text console) unchanged.
- Keep kernel log output on serial (unchanged).

## Approach

1. `crt0.S`: request a graphics framebuffer from GRUB by enabling the
   multiboot2 framebuffer request tag (type 5, 1024x768x32), and remove
   the unreachable post-`multiboot_main` drawing code.
2. `arch/multiboot.c`:
   - `multiboot_parse_framebuffer(boot_info_pa)` walks the multiboot2 tag
     list and, when a direct-RGB framebuffer tag (type 8) is present with
     a 32/24-bpp mode whose base address fits in 32 bits, records its
     base address, pitch, dimensions, bpp and channel positions.
   - `multiboot_fb_available()` / `multiboot_fb_fill(r,g,b)` expose the
     parsed framebuffer to the rest of the kernel.
3. `multiboot_main`: call `multiboot_parse_framebuffer` early. If a
   framebuffer is available, fill it with a solid colour (boot banner);
   otherwise fall back to `vga_console_init()`.
4. `core/misc.c` (`sys_panic`): if a framebuffer is available, fill it
   with red before halting, so fatal panics are visible on UEFI displays.
5. `core/domain.c` (`domain_panic`) is left unchanged (non-fatal).

## Non-goals

- Full text console on the UEFI framebuffer (serial remains the log
  channel).
- 64-bit framebuffer addresses (JCore is 32-bit; addresses above 4 GiB
  are ignored and the legacy path is used).
- Changes to the Java-side `FBEmulation` VESA path.

## Files touched

- `src/Assembly/crt0.S`
- `src/Headers/arch/multiboot.h`
- `src/arch/multiboot.c`
- `src/core/misc.c`
