# Mission: JVM Garbage Collection, Mastered in JCore's Own Code

## Why
JCore is a Java Virtual Machine the user is building and running on bare metal (ARM/x86), with real collectors already in `src/Memory/gc_*.c`. The user wants to understand JVM garbage collection deeply enough to reason about — and eventually tune or improve — JCore's heap, and to speak the same language as HotSpot's collectors when reading the wider JVM literature.

## Success looks like
- Trace JCore's copying collector (`gc_org.c`) end-to-end: what triggers a GC, what the root scan visits, how objects move, where the collector must pause and why.
- Explain any HotSpot collector (Serial, Parallel, G1, ZGC, Shenandoah) as an instance of the reachability model plus a strategy for finding/disposing of garbage.
- Compare JCore's collectors (`gc_compacting.c`, `gc_bitmap.c`, `gc_chunked.c`) against their HotSpot counterparts and name the trade-offs.
- Read a GC log or heap dump and diagnose why a full GC happened.
- Navigate a real JVM's heap layout (Eden/Survivor/Old) and predict which objects end up where.

## Constraints
- Applied, code-first learning style: every concept must be anchored to JCore source (`src/Memory/`, `src/malloc.c`).
- Concise lessons, completable in minutes, one tangible win each.
- Workspace currently holds JCore's real GC code — use it as the primary lab, not toy examples.

## Out of scope
- Writing a new collector from scratch (JCore already has several).
- Tuning production application heaps via JVM flags — relevant later only as diagnostic reading.
- JavaScript/Ruby/Python garbage collectors — JVM and JCore focus only.
