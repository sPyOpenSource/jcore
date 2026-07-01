# Mission: Teach x86-64 Long Mode Boot

## Objective
Teach the student how to transition from 32-bit protected mode to 64-bit long mode in an x86 operating system.

## Context
The student has an existing x86 OS codebase with a working bootloader. They want to understand the complete long mode entry sequence.

## Current Codebase State
- `src/loader.s` - Combined 32-bit boot + 64-bit stub (appears to be the current working version)
- herd- `src/boot32.s` - 32-bit boot code with page tables, GDT, and far jump
- `src/boot64.s` - 64-bit long mode start code
- `src/kernel.cpp` - C++ kernel entry point

## Learning Path
The teaching will cover:
1. Real mode vs Protected mode vs Long mode
2. Page table setup (PML4, PDPT, PD) for identity mapping
3. Enabling PAE (Physical Address Extension)
4. Enabling Long Mode via EFER MSR
5. Setting up a 64-bit GDT
6. The far jump to 64-bit code
7. 64-bit assembly stub and calling into C/C++
8. Verifying we are in long mode
9. Common pitfalls and debugging tips

## Verification
- Code compiles and runs in QEMU
- Kernel successfully enters long mode
- Can verify 64-bit mode is active
