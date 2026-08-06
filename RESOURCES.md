# Resources: JVM Garbage Collection

## Knowledge

- [Oracle GC Tuning Guide — Introduction to Garbage Collection Tuning](https://docs.oracle.com/en/java/javase/17/gctuning/introduction-garbage-collection-tuning.html)
  Official HotSpot overview of the reachability model and collector families. Use for: lesson 1 anchoring, refresh before reading any HotSpot code.
- [Oracle GC Tuning Guide — HotSpot Generations](https://docs.oracle.com/en/java/javase/17/gctuning/hotspot-garbage-collection.html)
  The Eden/Survivor/Old layout and why HotSpot is generational. Use for: the generational lesson.
- [Javadoc: `java.lang.ref` package summary](https://docs.oracle.com/en/java/javase/17/docs/api/java.base/java/lang/ref/package-summary.html)
  Spec-level definition of reachability levels (strong, soft, weak, phantom). Use for: precise language about what keeps an object alive.
- [JVM Anatomy Park (Shipilёv, OpenJDK)](https://shipilev.net/jvm/anatomy-quarks/)
  Short, surgical posts on JVM internals, many on GC behavior. Use for: resolving specific "but what actually happens?" questions.
- [G1: One Garbage Collector To Rule Them All (Oracle)](https://www.oracle.com/technical-resources/articles/java/g1gc.html)
  Clear introduction to the region-based G1 collector. Use for: comparing JCore's `gc_chunked.c` regions against G1.
- [The Z Garbage Collector (Azul/OpenJDK)](https://docs.oracle.com/en/java/javase/17/gctuning/implementing-garbage-collectors.html)
  Colored pointers, load barriers, concurrent relocation. Use for: the "how can GC run without stopping the world" lesson.
- [Book: _The Garbage Collection Handbook_ — Richard Jones, Antony Hosking, Eliot Moss (2nd ed.)](https://www.routledge.com/The-Garbage-Collection-Handbook-The-Art-of-Automatic-Memory-Management/Jones-Hosking-Moss/p/book/9780367415644)
  The definitive text. Chapter 1 covers the reachability model; later chapters cover copying, generational, and concurrent collectors. Use for: depth behind any lesson.
- [Cheney's algorithm (Wikipedia — traced to original 1970 CACM paper)](https://en.wikipedia.org/wiki/Cheney%27s_algorithm)
  Breadth-first copying collection, the exact shape JCore's `gc_org.c` uses. Use for: the copying-collector lesson.

## Wisdom (Communities)

- [r/java](https://reddit.com/r/java)
  High-signal for JVM ecosystem discussion; GC questions get serious answers. Use for: sanity-checking claims, seeing real production GC war stories.
- [r/programming](https://reddit.com/r/programming)
  Broad; good for papers and blog posts about collector design once past the basics.
- Local: the JCore codebase itself (`src/Memory/gc_*.c`)
  The most reliable "community" for this mission — JCore's collectors are the primary lab and ground truth.
