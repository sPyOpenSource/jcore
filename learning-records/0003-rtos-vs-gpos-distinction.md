# RTOS vs General-Purpose OS distinction established

The user asked "rtos vs general os" — a conceptual comparison that frames everything we're building. Key understanding: the difference is not hardware but commitment to **worst-case bounded latency** vs. **average-case throughput**. Three pillars: fixed-priority preemptive scheduling, bounded kernel preemption, and priority inheritance protocol for mutual exclusion.

The user's RTOS is a trust-based, fixed-priority preemptive RTOS with domain isolation — explicitly not trying to be Linux. This frames all future implementation decisions (static allocation, simple data structures, trust-based protection model).
