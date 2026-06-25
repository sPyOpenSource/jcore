# 0005: Kernel Memory Allocator (jxmalloc)

## Context
After the boot stages lesson, user asked about the `malloc` function in JCore. JCore has its own kernel-level allocator, not libc malloc.

## What we learned
- JCore uses a **bitmap-based block allocator** with 1024-byte fixed blocks (`BLOCKSIZE`)
- `jxmalloc_init()` determines the memory range from Multiboot2: from end of last boot module to top of RAM
- The bitmap is stored at the start of the managed memory range
- Allocation is first-fit with a rotating pointer (`current_block`) for fairness
- No size metadata is stored — `jxfree(addr, size)` requires the exact same size passed back
- Two-tier design: `jxmalloc` (global, block-granular) + `malloc_code` (per-domain bump allocator)
- `malloc_code` allocates from domain-specific code segments; no individual free, bulk-freed on domain termination
- Thread stacks use `malloc_threadstack` with alignment support
- Memory types (MEMTYPE_CODE, MEMTYPE_STACK, etc.) enable tracking statistics

## Key insights
- The allocator can't determine block count from address alone — must track size externally
- The two-tier design (global bitmap + per-domain bump) is a classic kernel pattern that avoids fragmentation in the hot path
- `jxmalloc_stat()` prints full allocator state including fragmentation

## Status
Accepted. Lesson 7 open in browser; pushed to `multiboot2`.
