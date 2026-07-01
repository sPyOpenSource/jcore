# Mission: TI Processor SDK for Baremetal OS Development on BeagleBone AI

## Why

I'm building a baremetal OS (JX) for the BeagleBone AI (AM5729). To do that effectively, I need to understand what TI officially provides — tools, documentation, bootloader sources, low-level drivers — so I can leverage the right parts instead of reinventing the wheel or working against the ecosystem.

## Success looks like

- Able to navigate the TI Processor SDK (Linux + RTOS) to find the right tool, doc, or example for a given task
- Understand the boot chain from TI ROM through SPL/MLO to U-Boot to custom kernel
- Know where to find AM5729 register definitions, device trees, and board init code without guessing
- Can build a minimal MLO + U-Boot from source if the pre-built ones don't work
- Know the difference between TI-supported and BBAI-board-specific code — what's reusable vs. what must be custom

## Constraints

- Developing on macOS (cross-compiling for ARM)
- No CCS/Windows dependency if possible — prefer open toolchain (clang/GCC, OpenOCD)
- Learning is driven by the real need to make the JX baremetal OS boot on hardware

## Out of scope

- Linux kernel internals or driver development (the goal is baremetal, not Linux)
- Yocto/Buildroot full distro building
- DSP or IPU (M4) programming — Cortex-A15 only for now
