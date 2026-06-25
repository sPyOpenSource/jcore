# Notes

## User preferences
- Wants concise, codebase-grounded lessons
- Learning style: applied, code-first (wants to see the concepts in their own assembly code)
- Topic request was: "about realmode and protectmode" → "figure out cons and pos" → "continue"
  - "cons and pos" interpreted as "concepts and positions" — the conceptual understanding of each mode and how they position relative to each other in the boot flow

## Session state (2026-06-25)
### Done
- Lesson 1: Real Mode — segment:offset, 1 MB limit, BIOS interrupts, `prot_to_real`/`vesa2_detect`
- Lesson 2: TCB — Ring 0/3 boundary, JCore TCB map, boot chain of trust
- Lesson 3: MMU — 32-bit page tables, TLB, page faults, CR0.PG, JCore's page fault handler
- Lesson 4: Long Mode — 64-bit vs 32-bit, 4-level paging, canonical addresses, EFER.LME
- Lesson 5: Disable Paging — identity mapping, triple fault, long→compat mode drop
- Lesson 6: Boot Stages — GRUB→crt0.S→multiboot_main→main→start_domain_zero→Init domain

### Next
- Ask user: GDT layout (`segments.h`, `asm.S` GDT entries), boot process deep dive (`crt0.S`), or JCore-specific portal/domain isolation pattern?
