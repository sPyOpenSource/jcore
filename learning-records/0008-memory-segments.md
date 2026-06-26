# Memory Segments: .bss, .data, Stack, Heap

## What happened
We created Lesson 8 covering the four memory territories in a running x86 kernel: `.data`, `.bss`, stack, and heap. The lesson maps each directly to JCore's `linker.ld`, `crt0.S`, `stack.S`, and `malloc.c`.

## Key insights
- `.bss` and `.data` are fixed-size regions determined at link time. Stack and heap are dynamic.
- `.bss` is zero-initialized by the bootloader (or startup code). JCore's `crt0.S` has the `.bss` clear **commented out**, relying on GRUB to do it.
- Critical difference: `.data` retains its values even when `.bss` is corrupted (which is why `ser_io_base[]` in `.data` survives the crash).
- The crash cascade: `.bss` zeroed → `jxmem_start = 0` → `jxmalloc` allocates from address 0 → thread stacks allocated near 0 → `PCB_ESP` becomes ~0 → ESP=0 in DomainZero → `zip` pointer in `.bss` is also zero → `zip_next_entry` PANICs.

## Associated lesson
`lessons/0008-memory-segments.html`

## Next steps
- Debug the `.bss` zeroing: insert a canary at a known `.bss` address and poll it after each `main()` function call
- Or: prevent the cascade by moving `zip`, `java_lang_Object`, and `jxmem_start`/`jxmem_end`/`bitmap` out of `.bss` by giving them explicit initializers (even `= 0` with `__attribute__((section(".data")))`)
