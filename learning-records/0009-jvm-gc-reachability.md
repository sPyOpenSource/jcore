# 0009: JVM Garbage Collection — the Reachability Model

## Context
User asked to learn "about gc of jvm". Workspace mission (x86 porting) is stale relative to the actual project, which is a JVM reimplementation (JCore). JCore already ships real collectors in `src/Memory/gc_*.c` — including a semispace copying collector (`gc_org.c`, 2269 lines) — so GC lessons can be fully grounded in their code. User learning style: concise, code-first, applied.

## What we learned
- GC is the *only* reclamation mechanism in the JVM: `new` never pairs with a `free`.
- **Garbage = unreachable from the root set.** Not program action, not a count — pure graph reachability.
- Consequences of the definition: no dangling pointers; unreachable cycles are collected (reference counting leaks exactly these); survivors may be *moved* (that's what copying GCs do).
- Root set: thread stacks, static fields, JNI refs, JIT registers. JCore's `gc_org_gc()` scans thread stacks then classes/statics — the same recipe as HotSpot, because JCore is a JVM.
- Every HotSpot collector (Serial/Parallel/CMS/G1/ZGC/Shenandoah) differs only in *how it finds and disposes of* unreachable objects; they share the reachability definition.
- Three families: mark-sweep, copying, mark-compact.
- Bump allocation is the trivial half of GC — the user already built one in `malloc_code` (Lesson 7); deciding what to reclaim is the hard half.

## Key insights
- The user's `malloc_code` bump allocator (allocate-only, bulk free on domain death) is the natural bridge into GC thinking.
- `gc_org_allocHeap2` (gc_org.c:62) is literally a bump pointer into the to-space — the same code shape they know from the kernel.
- JCore's collectors are a live taxonomy: `gc_org.c`=copying, `gc_compacting.c`=mark-compact, `gc_bitmap.c`=bitmap, `gc_chunked.c`=regions (G1 ancestor).
- Stack scanning requires knowing which words are pointers → stack maps (teaser for next lesson).

## Next steps
- Lesson 2: the copying/semispace collector, walking `gc_org_gc()` end to end (from-space → to-space, forwarding pointers, why allocation is a bump pointer).
- Lesson 3: HotSpot's generational heap (Eden/Survivor/Old) and why the weak generational hypothesis holds.
- Then: JCore `gc_compacting.c` vs HotSpot mark-compact; G1/ZGC concurrency; safepoints.

## Status
Accepted. Lesson 9 open in browser. MISSION.md rewritten to reflect the actual project (JVM reimplementation / JCore GC mastery); RESOURCES.md repopulated for the GC thread. User confirmed next lesson: copying collector in `gc_org.c` (GC II).
