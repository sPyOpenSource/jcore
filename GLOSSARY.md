# TI Processor SDK Glossary

Key terminology for navigating TI's embedded software ecosystem. All lessons in this workspace adhere to these definitions.

## SDK Components

**Processor SDK (Linux)**:
The full Linux distribution package from TI for a given SoC family (AM57x). Includes kernel, U-Boot, toolchain, rootfs, and flashing scripts. Purpose-built for Linux application development.
_Avoid_: "the SDK" without qualifier

**Processor SDK (RTOS)**:
The baremetal/RTOS development package from TI. Includes CSL, PDK low-level drivers, SYS/BIOS RTOS, IPC, and example projects. This is the relevant SDK for baremetal OS work.

**PDK (Platform Development Kit)**:
The hardware abstraction layer within the RTOS SDK. Provides C headers for all SoC registers (CSL), peripheral driver libraries (LLD), and board initialization code. The closest thing TI offers to a "baremetal SDK."

**CSL (Chip Support Library)**:
Register-level header files and macros inside the PDK. `CSL_uartRegs`, `CSL_gpioRegs`, etc. Directly maps to TRM memory maps.

**LLD (Low-Level Driver)**:
Higher-level peripheral driver built on top of CSL. Handles FIFO management, interrupt dispatching, DMA setup. Optional — you can use CSL directly.

## Boot Chain

**ROM Bootloader**:
The mask-ROM code in the AM5729 that runs on reset. Reads boot mode pins (or EEPROM), loads the next stage from SD, eMMC, UART, or USB. Immutable and documented in the TRM.

**MLO (aka SPL — Secondary Program Loader)**:
The first software-loaded stage. A small binary built from U-Boot SPL, prepended with a TI Configuration Header (CH). On AM57xx, loaded by ROM from SD card raw sector 512 (or eMMC boot partition at sector 256).

**Configuration Header (CH)**:
A 512-byte header preceding the SPL binary in MLO. Contains image size, entry point, and the `CHSETTINGS` magic string. Injected by `mkimage` or the U-Boot build system.

**U-Boot (proper)**:
The second-stage bootloader, loaded by MLO from a FAT partition as `u-boot.img`. Provides a shell, filesystem access, network boot, and chainloading capability.

**FIT (Flattened Image Tree)**:
A device-tree-based container format that wraps U-Boot proper, device tree blobs, and additional firmware into a single `u-boot.img` file. Starts with FDT magic (`d0 0d fe ed`).

**Chainloading**:
Using one bootloader to load and jump to another. In our setup: ROM → MLO (SPL) → U-Boot → `go 0x80200000` (our kernel).

## Hardware Reference

**TRM (Technical Reference Manual)**:
The 7000+ page definitive register-level reference for the AM5729. Document number SPRUHZ6 (Rev L). Contains memory maps, register descriptions, boot ROM behavior, and peripheral programming guides.

**Device Tree (DTB / DTS)**:
A data structure for describing hardware — not for baremetal, but essential for understanding U-Boot and Linux board support. The BBAI's device tree is `am5729-beagleboneai.dts`.

**Pin Mux**:
The AM5729 has multiple functions per physical pin, selected by the Control Module registers. The TI Pin Mux Tool provides a GUI for generating the correct register values.

## Memory Map

**OCMC_RAM (On-Chip Memory)**:
256 KB of internal SRAM (at 0x40300000). Used by the ROM and early SPL stages before DDR is initialized.

**DDR3L (External Memory)**:
The BBAI has 1 GB of DDR3L (two 512 MB MT41K512M16HA-125 chips) at a 533 MHz clock. Mapped starting at 0x80000000. Initialized by the SPL.

**Load Address**:
Where U-Boot places the kernel in DDR. For the BBAI: `0x80200000` (standard for TI ARM32 boot convention).

## Development Tools

**create-sdcard.sh**:
Script bundled in the Linux SDK that formats an SD card with the correct partition layout (FAT32 boot + ext4 rootfs) and copies MLO, u-boot.img, and kernel. Useful reference for manual SD card setup.

**CCS (Code Composer Studio)**:
TI's Eclipse-based IDE with integrated debugger. Supports JTAG debugging of the AM5729 via the built-in ICEmelter (ICED) interface over USB. Optional — can work without it.

**ICED (ICEmelter Debug)**:
The built-in JTAG debug controller in the AM5729, accessible via USB. Supported by OpenOCD and CCS.
