# BBAI SDK & Baremetal Resources

## Knowledge

- [AM5729 TRM (SPRUHZ6 Rev L)](https://www.ti.com/lit/pdf/spruhz6)
  The definitive 7000-page register-level reference. **Primary source for all register addresses, bit fields, boot ROM behavior, and peripheral programming.** Use for: looking up UART/GPIO/DDR register offsets, understanding the boot sequence, finding the memory map.

- [AM5729 Datasheet (SPRS953G)](https://www.ti.com/lit/gpn/AM5729)
  Electrical characteristics, pinout, package info. Use for: pin assignments, power sequencing, timing parameters for DDR init.

- [Processor SDK RTOS for AM57X — Download](https://www.ti.com/tool/PROCESSOR-SDK-RTOS-AM57X)
  The baremetal/RTOS development kit. Contains CSL headers, PDK drivers, SYS/BIOS, and example projects. **The main SDK for baremetal development on the AM5729 SoC.**

- [Processor SDK RTOS Documentation](https://software-dl.ti.com/processor-sdk-rtos/esd/AM57X/11_01_00_00/exports/docs/rtos/index_overview.html)
  API docs for PDK, CSL, LLD, and board library. Use for: finding UART init functions, GPIO toggle API, interrupt setup.

- [Processor SDK Linux for AM57X — Download](https://www.ti.com/tool/PROCESSOR-SDK-AM57X)
  The full Linux SDK. Useful even for baremetal work for: `create-sdcard.sh` reference, U-Boot source, device trees, and the ARM GCC toolchain.

- [BeagleBoard.org — BeagleBone AI Docs](https://docs.beagleboard.org/boards/beaglebone/ai/)
  Board-specific documentation: pin headers, capes, known issues. Use for: which UART is the console, LED GPIO numbers, SD card boot procedure.

- [BeagleBone AI Hardware Design Files (GitHub)](https://github.com/beagleboard/beaglebone-ai)
  Schematics, PCB layout, BOM. Use for: verifying GPIO assignments, DDR part numbers, power rail voltages, pin connection details not in the TRM.

- [U-Boot Source (BeagleBoard fork)](https://github.com/beagleboard/u-boot)
  The board-specific U-Boot fork for the BBAI. Use for: reference SPL board init code, pinmux setup, DDR timing configuration for the BBAI's specific DDR3L chips.

- [bb-kernel (Robert Nelson's repo)](https://github.com/RobertCNelson/bb-kernel)
  Build scripts and kernel sources for BeagleBoard Debian images. Use for: the BBAI device tree (`am5729-beagleboneai.dts`) and kernel config as reference.

- [TI Pin Mux Tool for AM57x](http://software-dl.ti.com/processor-sdk-linux/esd/AM57X/latest/index_FDS.html)
  GUI tool for configuring pin multiplexing. Use for: generating the correct register values for pinmux configuration in baremetal board init code.

- [OpenOCD — TI ICEDi Support](https://openocd.org/)
  Open-source JTAG debugger. Supports the AM5729's built-in ICEmelter (ICED) debug interface over USB. Use for: baremetal debugging without CCS.

- [TI Sitara Academy (AM57x module)](https://dev.ti.com/tirex/global?id=com.ti.JACINTO_ACADEMY_AM574X)
  Video training series on AM57x architecture and programming. Use for: conceptual understanding of the SoC architecture, clock tree, DMA, PRU.

- [BeagleBoard Image Builder](https://github.com/beagleboard/image-builder)
  Scripts that generate the official BeagleBoard SD card images. Use for: understanding how the boot partition is laid out, which U-Boot config is used.

## Gaps

- No single "BBAI baremetal board file" exists — the BBAI is officially community-supported. You must write `board_init.c` yourself, using the TI EVM board files as reference. The BBAI schematics and the BeagleBoard U-Boot fork's `board/ti/am57xx/board.c` are the closest substitutes.
- No official TI-RTOS BSP for the BBAI board (only for TI EVM/IDK boards).
- No pre-built PDK for the BBAI — the CSL is SoC-level (AM5729) and reusable, but the board-level HAL (pinmux, clock tree, DDR config) needs to be handwritten for the BBAI.

## Wisdom (Communities)

- [TI E2E Forums — Sitara](https://e2e.ti.com/support/processors-group/)
  Official TI support forum. Use for: TRM clarification, errata workarounds, toolchain issues. Search before posting — many AM57x questions have been answered.

- [BeagleBoard Forum](https://forum.beagleboard.org/)
  Community forum for all BeagleBoard products. Use for: BBAI-specific questions, boot issues on real hardware, capes compatibility.

- [BeagleBoard Matrix Chat](https://chat.beagleboard.org/)
  Real-time chat with BBAI community members and developers. Use for: quick questions, build troubleshooting, hardware debugging tips.

- [u-boot Mailing List](https://lists.denx.de/listinfo/u-boot)
  Main U-Boot development list. Use for: SPL/boot flow questions, U-Boot porting issues (filter by "am57xx" or "omap").
