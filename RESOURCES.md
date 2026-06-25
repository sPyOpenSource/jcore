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

## Wisdom (Communities)
- [r/osdev](https://reddit.com/r/osdev) — active community of OS developers, good for mode-switching questions
- [OSDev Forum](https://forum.osdev.org) — more focused, senior developers hang out there
