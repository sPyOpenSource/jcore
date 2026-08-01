# Design: Restructure `src/` into three buckets

Date: 2026-08-01
Status: Approved (design review)

## Goal

Reorganize the flat `src/` directory of the jcore kernel into three coarse
subsystem buckets (`arch/`, `core/`, `debug/`) so the filesystem reflects how
the build already classifies the code. Purely a file move — no kernel logic
changes.

## Scope

- `src/` only. The submodules (`Zero/`, `OS/`, `Compiler/`, `AIZero/`, `test/`)
  and all top-level files outside `src/` are untouched.
- The 19 stray `.ll` files in `src/` are deleted and `*.ll` added to
  `.gitignore`.

## Target layout

All 52 top-level `.c` files move into one of three buckets. `src/Interface/`,
`src/Memory/`, and `src/Assembly/` stay as-is.

```
src/
  arch/    hardware + boot glue
           minic.c  multiboot.c  irq.c  lapic.c  io_apic.c
           smp.c  smp_detect.c  smp_activate.c  timer8254.c  cpuid.c

  core/    runtime + sched + memory + util
           main.c  classes.c  load.c  execJAVA.c  libcache.c  portal.c
           domain.c  misc.c  atomic.c  atomicfn.c  exception_handler.c
           oneshot.c  syscalls.c
           thread.c  interrupt.c  javascheduler.c  msg.c  runq.c
           runq_inline.c  sched_global.c  sched_global_rr.c  scheduler_global.c
           sched_local.c  sched_local_rr.c  sched_local_java.c
           malloc.c  memfs.c  vmsupport.c
           zip.c  minilzo.c  crc32.c  bdt.c  ekhz.c  math.c

  debug/   profile.c  monitor.c  thread_debug.c  thread_emulation.c
           thread_profile.c  serialdbg.c  symfind.c  minic_debug.c

  Headers/  subdivided (see below)
  Interface/  unchanged
  Memory/     unchanged
  Assembly/   unchanged
```

## Headers

`src/Headers/` gets the same three buckets. Root of `Headers/` keeps the shared
and generated headers. `-Isrc/Headers` remains the single include root.

```
src/Headers/
  all.h  config.h  types.h  misc.h  symbols.h  realmode.h     # root
  arch/   minic.h  multiboot.h  irq.h  lapic.h  io_apic.h
          smp.h  smp_detect.h  timer8254.h  cpuid.h  intr.h
          lowlevel.h  segments.h  context.h
  core/   thread.h  threadimpl.h  sched.h  runq.h  scheduler_inlined.h
          interrupt.h  javascheduler.h  spinlock.h  lock.h  msg.h
          classes.h  load.h  execJAVA.h  portal.h  portal_proto.h  domain.h
          libcache.h  atomic.h  atomicfn.h  exception_handler.h  code.h
          object.h  syscalls.h  malloc.h  malloc_proto.h  memfs.h
          vmsupport.h  zip.h  minilzo.h  lzoconf.h  crc32.h  bdt.h  ekhz.h
  debug/  profile.h  monitor.h  serialdbg.h  debug_reg.h  bench.h  symfind.h
```

Include rules:

- Moved headers are referenced as `#include "core/thread.h"`,
  `#include "arch/lapic.h"`, etc.
- Every `#include "x.h"` for a moved header is updated in both `.c` and `.h`
  files. Root headers and the existing `"Memory/..."` / `"Interface/..."`
  paths are untouched.
- Both `-Isrc` and `-Isrc/Headers` flags stay as they are; no include-path
  changes.

## Build system

- Rewrite the source lists in `Makefile` (`ESSENTIALSOURCES`,
  `SUPPORTSOURCES`, `ESSENTIALCORESOURCES`, `CORESOURCES`, `INCLUDES`) to the
  new `core/`, `arch/`, `debug/` prefixes.
- Extend the object-flattening substitutions (`COREOBJ2`/`COREOBJ3`) to also
  strip `core/`, `arch/`, `debug/` so `.kernel/` objects stay flat, exactly as
  today.
- The `.kernel/%.o: src/%.c` pattern rule already matches subdirectories; no
  rule changes.

## Verification

- `make` must still produce `jxcore` (this is what CI runs on ubuntu-latest).
- The `.ll` files are gone and `*.ll` is gitignored.

## Known side effects (out of scope)

- Lesson HTML files under `lessons/` reference old `src/...` paths in prose;
  they may become slightly stale. Not updated in this change.
- Six `.c` files were already absent from the Makefile before this change:
  `javascheduler.c`, `math.c`, `minic_debug.c`, `msg.c`, `runq_inline.c`,
  `scheduler_global.c`. They move to their buckets but remain unbuilt, as today.

## Execution

A single move + rename commit. No kernel logic is touched.
