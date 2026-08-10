# 0011: GC Optimization — Trade-offs and Log Analysis

## Context
User asked to learn "optimalization" (optimization) in the context of JVM garbage collection. This is Lesson 11 in the GC thread, building on Lesson 9 (reachability model) and Lesson 10 (scheduler).

## What we learned
- **GC optimization is a three-way trade-off**: pause time (latency) vs throughput vs memory footprint. You cannot optimize all three simultaneously.
- **Four JCore collectors map to different corners**:
  - `gc_org` (semispace copying): low pause, good throughput, 2× heap → like HotSpot Serial Young
  - `gc_compacting` (mark-compact): longer pause, lower throughput, 1× heap → like HotSpot Serial Old
  - `gc_bitmap` (bitmap mark-sweep): variable pause, good throughput, 1× heap + bitmap → CMS-style
  - `gc_chunked` (region-based): incremental pause, good throughput, flexible → G1 regions
- **Key metrics from JCore GC logs**: pause time (`Time`), collected bytes, allocation rate, throughput %, survival rate
- **Diagnosing patterns**: growing pauses = heap too small or leak; periodic huge pauses = full GC / promotion failure; rare spikes = incremental collector doing full compaction

## Key insights
- The "optimal" collector depends entirely on workload: batch jobs want throughput, interactive apps want low pause, embedded wants minimal footprint
- Your allocation rate is the primary driver — if it doubles, you need 2× heap or 2× pause
- JCore's `printInfo` output for each collector gives you the raw data; the skill is computing derived metrics and spotting trends
- HotSpot's collectors (Serial, Parallel, G1, ZGC, Shenandoah) are different engineering answers to the same three-way trade-off

## Next steps
- Lesson 12: Deep dive into `gc_org.c` — walking the semispace collector end-to-end (from-space → to-space, forwarding pointers, why allocation is a bump pointer)
- Then: HotSpot's generational heap (Eden/Survivor/Old) and why the weak generational hypothesis holds
- Then: JCore `gc_compacting.c` vs HotSpot mark-compact; G1/ZGC concurrency; safepoints

## Status
Accepted. Lesson 11 created at `lessons/0011-gc-optimization.html`. User confirmed they want to proceed to the copying collector deep dive next.