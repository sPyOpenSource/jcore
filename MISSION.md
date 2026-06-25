# Mission: x86 Real Mode + Protected Mode for OS Development

## Why
You're building the JCore (JX) OS kernel — a Java-executing x86 OS. Your kernel boots via GRUB (which starts in protected mode), but it also has real-mode switching code for VESA BIOS calls and ACPI. You need to understand both modes deeply to maintain and extend this code: why the CPU starts in real mode, how the transition to protected mode works, and when you must switch back.

## Success looks like
- Trace the boot path from CPU reset → real mode → GRUB → protected mode → JCore kernel
- Explain what each GDT entry in `segments.h` does, including the 16-bit segments
- Read `asm.S` (`real_to_prot` / `prot_to_real`) and explain every instruction's role
- Know when and why the kernel drops back to real mode (VESA, ACPI, SMP startup)
- Understand the limitations that motivate the next transition (long mode on x86-64)

## Constraints
- Focus on 32-bit protected mode (that's what JCore uses now); long mode is future scope
- Tie every concept back to the JCore codebase — this is applied knowledge, not theory
- Lessons should be short and completable in one sitting

## Out of scope
- Java bytecode execution, class loading, JCore's Java internals (that's the `OS/` workspace)
- ARM processor modes (this is x86-specific)
- CPUID, ACPI table parsing, or other advanced system programming topics
