# x86 Real Mode + Protected Mode Resources

## Knowledge

### Primary (Intel Manuals)
- [Intel 64 and IA-32 Architectures Software Developer's Manual, Vol. 3A: System Programming Guide](https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html)
  Chapters 2-4: processor modes, segmentation, protection. The ultimate authority.
  Use for: any question about GDT, segment descriptors, control registers, mode transitions.

### Primary (OSDev Wiki — the standard reference for OS developers)
- [Real Mode](https://wiki.osdev.org/Real_Mode) — comprehensive: addressing, limitations, switching
- [Protected Mode](https://wiki.osdev.org/Protected_Mode) — entering PM, GDT setup, ring model
- [Virtual 8086 Mode](https://wiki.osdev.org/Virtual_8086_Mode) — how PM OSes run RM code safely

### Secondary (Tutorials)
- [The World of Protected Mode](http://www.osdever.net/tutorials/view/the-world-of-protected-mode) (osdever.net) — classic tutorial on entering PM
- [BrokenThorn OSDev: Protected Mode](http://www.brokenthorn.com/Resources/OSDev8.html) — step-by-step GDT + PM entry with diagrams
- [Wasil Zafar: Kernel Development Series](https://www.wasilzafar.com/pages/series/kernel-development/) — Phase 2 (Real Mode) + Phase 3 (Protected Mode), well-written, 2026
- [Protected Mode Programming Tutorial](https://prodebug.sourceforge.net/pmtut.html) (SourceForge) — thorough, includes V86 mode and multitasking

### This Codebase
- `src/Assembly/asm.S` — `real_to_prot` / `prot_to_real` mode switching functions + GDT definition
- `src/Assembly/crt0.S` — 32-bit protected mode entry point via Multiboot2
- `src/Headers/segments.h` — GDT selector constants (`KERNEL_CS`, `KERNEL_16_CS`, etc.)
- `src/main.c` — kernel main, calls `pic_init_pmode()`, comments about `init_realmode()`

## Wisdom (Communities)
### Trusted Computing Base
- [Wikipedia: Trusted Computing Base](https://en.wikipedia.org/wiki/Trusted_computing_base) — solid overview, traces back to Rushby and the Orange Book
- [CodeLucky: TCB Security Kernel Design](https://codelucky.com/trusted-computing-base-security-kernel/) — practical architectural guide
- [NIST Glossary: TCB](https://csrc.nist.gov/glossary/term/trusted_computing_base) — the formal definition
- [Ken Thompson: Reflections on Trusting Trust](https://en.wikipedia.org/wiki/Reflections_on_Trusting_Trust) — why the compiler is in the TCB
- [seL4 Microkernel](https://sel4.systems/) — formally verified TCB, proof that small TCBs can be audited to zero bugs

### Memory Management Unit
- [OSDev Wiki: Paging](https://wiki.osdev.org/Paging) — practical how-to for setting up page tables on x86
- [OSDev Wiki: Memory Management Unit](https://wiki.osdev.org/Memory_Management_Unit) — conceptual deep dive on MMU and TLB
- [Wikipedia: Memory Management Unit](https://en.wikipedia.org/wiki/Memory_management_unit) — broad overview
- [Intel SDM Vol. 3A, Chapter 4: Paging](https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html) — authoritative reference
- [Virtual Memory: Page Tables, TLBs, and Linux Internals](https://blog.codingconfessions.com/p/virtual-memory) — excellent deep explanation with Linux specifics
- [MMU Handbook](https://kalairajah-personal.github.io/the-mmu-handbook)

### Long Mode (x86-64)
- [OSDev Wiki: x86-64](https://wiki.osdev.org/X86-64) — comprehensive overview of long mode features
- [OSDev Wiki: Setting Up Long Mode](https://wiki.osdev.org/Setting_Up_Long_Mode) — step-by-step assembly code
- [OSDev Wiki: CPU Registers x86-64](https://wiki.osdev.org/CPU_Registers_x86-64) — CR0-CR8, MSRs, EFER
- [Wikipedia: Long Mode](https://en.wikipedia.org/wiki/Long_mode) — history and architecture overview
- [Wasil Zafar: x64 Long Mode & Protected Mode](https://www.wasilzafar.com/pages/series/assembly-mastery/asm-part16-long-mode.html) — accessible tutorial with mode transition code
- [Linux x86-64 Boot Sequence](https://kernel-internals.org/arch/x86/boot)

### Disabling Paging / Mode Switching
- [OSDev Wiki: Triple Fault](https://wiki.osdev.org/Triple_Fault) — what happens when mode transitions go wrong
- [OSDev Wiki: Identity Paging](https://wiki.osdev.org/Identity_Paging) — why identity mapping is necessary for paging transitions
- [OSDev Forum: Return to real mode from long mode and back](https://forum.osdev.org/viewtopic.php?t=43127) — practical discussion of long→compat→prot→real chain
- Intel SDM Vol. 3A, Section 9.9: "Mode Switching" — complete specification for all mode transitions — real-world real→protected→long mode chain — rigorous, implementation-level reference across x86-64, ARM64, RISC-V (21 chapters), proof that small TCBs can be audited to zero bugs

## Wisdom (Communities)
- [r/osdev](https://reddit.com/r/osdev) — active community of OS developers, good for mode-switching questions
- [OSDev Forum](https://forum.osdev.org) — more focused, senior developers hang out there
