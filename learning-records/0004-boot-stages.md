# 0004: JCore Boot Stages

## Context
After five lessons on individual x86 features (real mode, TCB, MMU, long mode, disabling paging), the user wanted to see how it all fits together in JCore's actual boot path.

## What we learned
JCore's boot is a strict 5-stage pipeline, each enabling the next:

1. **GRUB** (not JCore code): CPU reset → real mode → BIOS → GRUB → protected mode → Multiboot2 handoff
2. **Assembly entry** (`crt0.S`): stack setup, preserve multiboot info, optional framebuffer test
3. **C low-level init** (`multiboot_main`): GDT (replaces GRUB's), IDT, debug regs, heap allocator
4. **Kernel subsystem init** (`main`): PIC, IRQ data, domain system (creates DomainZero), atomic ops, threads, portals, Java runtime primitives
5. **DomainZero Java thread** (`start_domain_zero`): APIC, timer, load `zero.jll` + `jdk0.jll`, initialize ~25 portals, create Init domain
6. **Init domain** (Java): scheduler, user domains

Key insight: JCore's boot path does **not** enable paging (`CR0.PG = 0` throughout) — it uses pure segmentation.

## Key decisions
- The boot path is a strict partial order; each stage depends on the previous
- `crt0.S` is minimal (~30 lines) — no GDT, paging, or interrupt setup of its own
- The commented-out `init_realmode()` in `main.c:129` would have injected a real-mode round-trip for VESA framebuffer detection between stages 3 and 4
- The handoff from C kernel to Java is via `start_domain_zero`, which loads JLL libraries and creates the Init domain

## Consequences
- Starting point for understanding where to add new kernel features
- Debugging early boot failures: isolate which stage crashes
- Connects all previous lessons into one coherent timeline

## Status
Accepted. Lesson 6 open in browser; pushed to `multiboot2`.
