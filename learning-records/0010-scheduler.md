# 0010: The Scheduler — Two-Level Round-Robin Runqueue

## Context
User asked to learn "about scheduler". Ad-hoc topic extension: the scheduler is the ground layer for the GC mission (safepoints, stopping mutators, scheduler-held roots), so it fits the mission even though it is not itself GC. Grounded entirely in JCore's own scheduler code.

## What we learned
- A scheduler answers one question — which runnable thread runs next — and is *event-driven*: created, unblocked, yielded, blocked/destroyed, timer-preempted. JCore entry points are `Sched_*()` in `src/core/runq.c`.
- Central invariant: only `STATE_RUNNABLE` threads live in a runqueue (`thread.h:20`). Blocking removes, unblocking re-adds.
- Two-level round-robin: global `domain_runqueue` (FIFO of runnable domains) + per-domain `firstThreadInRunQueue` (FIFO of threads). `runqueue_removeFirstOrNULL()` (runq.c:301) picks head domain then head thread. Domains are scheduled as units → one domain can't starve the whole system.
- The switch is assembly: `internal_switch_to` / `destroy_switch_to` in `src/Assembly/switch.S` save/restore EIP/ESP/EBP/regs into the thread's `context[]` PCB and `ret` into the stored EIP — the thread resumes mid-instruction.
- Idle thread: `sti; hlt; cli` parks the CPU until an interrupt.
- NEW_SCHED variant (`sched_global_rr.c`) adds pluggable per-domain local schedulers and the GC interface `Sched_gc_rootSet()` / `Sched_gc_tcb()`.
- JAVASCHEDULER mode: both LowLevel and HighLevel schedulers are Java objects living in the heap → the GC must treat them as roots and relocate them (`gc_org_moveSchedulerObject`, gc_org.c:48).

## Key insights
- The runqueue stores its links inside the domain/thread structs — no heap calls in the scheduling hot path, exactly like the "embedded next-pointer" style seen in the allocator lessons.
- GC and scheduler are entangled three ways: scheduler refs are GC roots, scheduler Java objects get moved, and GC can only run when threads are stopped (safepoints — which the scheduler controls).
- `STATE_AVAILABLE` is how the GC knows a thread need not be scanned (`gc_org.c:1164` skips those stacks) — states are a contract between scheduler and GC.
- JCore ships two scheduler generations (runq.c two-level, sched_global_rr.c pluggable) behind `NEW_SCHED`/`JAVASCHEDULER` switches — good example of evolution by feature flags.

## Next steps
- Safepoints: how JCore stops mutators so the GC can run (bridges scheduler → GC mission directly).
- The Java HighLevelScheduler (`sched_local_java.c`, `javascheduler.c` HLS side) — scheduling policy written in Java.
- Compare with OSTEP policies (RR vs SJF vs lottery) to classify JCore's choices.
- Resume GC thread: semispace copying collector walk of `gc_org_gc()` (was promised as GC II).

## Status
Accepted. Lesson 10 open in browser. Reference: `reference/scheduler.html`.
