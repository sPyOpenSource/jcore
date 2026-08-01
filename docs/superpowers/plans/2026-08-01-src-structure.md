# src/ Restructure Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Move the flat `src/` C sources and headers into three subsystem buckets (`arch/`, `core/`, `debug/`) without changing any kernel logic.

**Architecture:** Pure file renames via `git mv` (preserves history) plus mechanical `#include` path updates. The Makefile's existing source lists are rewritten to the new prefixes, and its object-flattening substitutions are extended to the three new directories so `.kernel/` object paths stay flat exactly as today. The pass/fail gate is a successful `make jxcore`.

**Tech Stack:** C, x86 assembly, GNU Make, clang/ld.lld toolchain (macOS dev host; CI runs `make` on ubuntu-latest).

## Global Constraints

- No kernel logic changes — moves and include-path rewrites only.
- Every source file currently in `src/` moves to exactly one of `src/arch/`, `src/core/`, `src/debug/` (full mapping verified against disk: 52 `.c`, 58 `.h`).
- `src/Interface/`, `src/Memory/`, `src/Assembly/` and their contents are untouched.
- Headers staying in `src/Headers/` root: `all.h`, `config.h`, `types.h`, `misc.h`, `symbols.h`, `realmode.h`.
- Include flags `-Isrc` and `-Isrc/Headers` are unchanged.
- Build gate: `make jxcore` must produce `jxcore`.
- The 19 stray `.ll` files are deleted and `*.ll` is gitignored.
- Single commit at the end. Intermediate states do not build, so no intermediate commits (a commit of a broken tree is worse than one atomic refactor commit).

---

### Task 1: Move `.c` sources into buckets

**Files:**
- Modify: (git moves) 52 files under `src/` → `src/arch/`, `src/core/`, `src/debug/`

**Interfaces:**
- Consumes: nothing.
- Produces: source files now located at `src/<bucket>/<file>.c`. Header `#include`s still resolve (headers are not moved yet and `-Isrc/Headers` is unchanged). The build is intentionally broken here because the Makefile still lists flat paths.

- [ ] **Step 1: Create the three bucket directories**

```bash
mkdir -p src/arch src/core src/debug
```

- [ ] **Step 2: Move the 10 arch sources**

```bash
git mv src/minic.c src/multiboot.c src/irq.c src/lapic.c src/io_apic.c \
       src/smp_detect.c src/smp_activate.c src/smp.c src/timer8254.c src/cpuid.c \
       src/arch/
```

- [ ] **Step 3: Move the 34 core sources**

```bash
git mv src/main.c src/classes.c src/load.c src/execJAVA.c src/libcache.c \
       src/portal.c src/domain.c src/misc.c src/atomic.c src/atomicfn.c \
       src/exception_handler.c src/oneshot.c src/syscalls.c \
       src/thread.c src/interrupt.c src/javascheduler.c src/msg.c src/runq.c \
       src/runq_inline.c src/sched_global.c src/sched_global_rr.c \
       src/scheduler_global.c src/sched_local.c src/sched_local_rr.c \
       src/sched_local_java.c \
       src/malloc.c src/memfs.c src/vmsupport.c \
       src/zip.c src/minilzo.c src/crc32.c src/bdt.c src/ekhz.c src/math.c \
       src/core/
```

- [ ] **Step 4: Move the 8 debug sources**

```bash
git mv src/profile.c src/monitor.c src/thread_debug.c src/thread_emulation.c \
       src/thread_profile.c src/serialdbg.c src/symfind.c src/minic_debug.c \
       src/debug/
```

- [ ] **Step 5: Verify — the only `.c` files left at `src/` root are inside `Interface/`/`Memory/`**

```bash
ls src/*.c
```

Expected: `zsh: no matches found: src/*.c` (i.e., no top-level `.c` files remain).
Verify bucket counts:

```bash
ls src/arch/*.c | wc -l   # 10
ls src/core/*.c | wc -l   # 34
ls src/debug/*.c | wc -l  # 8
```

---

### Task 2: Move headers into `Headers/` sub-buckets

**Files:**
- Modify: (git moves) 52 files under `src/Headers/` → `src/Headers/arch/`, `src/Headers/core/`, `src/Headers/debug/`

**Interfaces:**
- Consumes: Task 1 layout.
- Produces: moved headers at `src/Headers/<bucket>/<file>.h`. Root headers (`all.h`, `config.h`, `types.h`, `misc.h`, `symbols.h`, `realmode.h`) remain at `src/Headers/`. All `#include "x.h"` references are broken here (fixed in Task 3).

- [ ] **Step 1: Create the header bucket directories**

```bash
mkdir -p src/Headers/arch src/Headers/core src/Headers/debug
```

- [ ] **Step 2: Move the 13 arch headers**

```bash
git mv src/Headers/minic.h src/Headers/multiboot.h src/Headers/irq.h \
       src/Headers/lapic.h src/Headers/io_apic.h src/Headers/smp.h \
       src/Headers/smp_detect.h src/Headers/timer8254.h src/Headers/cpuid.h \
       src/Headers/intr.h src/Headers/lowlevel.h src/Headers/segments.h \
       src/Headers/context.h \
       src/Headers/arch/
```

- [ ] **Step 3: Move the 33 core headers**

```bash
git mv src/Headers/thread.h src/Headers/threadimpl.h src/Headers/sched.h \
       src/Headers/runq.h src/Headers/scheduler_inlined.h \
       src/Headers/interrupt.h src/Headers/javascheduler.h \
       src/Headers/spinlock.h src/Headers/lock.h src/Headers/msg.h \
       src/Headers/classes.h src/Headers/load.h src/Headers/execJAVA.h \
       src/Headers/portal.h src/Headers/portal_proto.h src/Headers/domain.h \
       src/Headers/libcache.h src/Headers/atomic.h src/Headers/atomicfn.h \
       src/Headers/exception_handler.h src/Headers/code.h src/Headers/object.h \
       src/Headers/syscalls.h src/Headers/malloc.h src/Headers/malloc_proto.h \
       src/Headers/memfs.h src/Headers/vmsupport.h src/Headers/zip.h \
       src/Headers/minilzo.h src/Headers/lzoconf.h src/Headers/crc32.h \
       src/Headers/bdt.h src/Headers/ekhz.h \
       src/Headers/core/
```

- [ ] **Step 4: Move the 6 debug headers**

```bash
git mv src/Headers/profile.h src/Headers/monitor.h src/Headers/serialdbg.h \
       src/Headers/debug_reg.h src/Headers/bench.h src/Headers/symfind.h \
       src/Headers/debug/
```

- [ ] **Step 5: Verify — only the 6 root headers remain at `src/Headers/` root**

```bash
ls src/Headers/*.h
```

Expected: exactly `all.h  config.h  misc.h  realmode.h  symbols.h  types.h`.
Verify bucket counts:

```bash
ls src/Headers/arch/*.h  | wc -l  # 13
ls src/Headers/core/*.h  | wc -l  # 33
ls src/Headers/debug/*.h | wc -l  # 6
```

---

### Task 3: Rewrite `#include` directives for moved headers

**Files:**
- Modify: every `.c`, `.h`, `.S` under `src/` that includes a moved header by bare name.

**Interfaces:**
- Consumes: Task 2 header locations.
- Produces: all references to moved headers use `"<bucket>/<name>.h"` form. `syscalls.c`'s `<syscalls.h>` becomes `"core/syscalls.h"`.

- [ ] **Step 1: Run the include-rewrite script**

Save the following as `rewrite_includes.sh` in the repo root, then run it:

```bash
#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/src"

rewrite() {
  local bucket="$1"; shift
  for h in "$@"; do
    for f in $(grep -rl "#include \"$h\"" --include='*.c' --include='*.h' --include='*.S' .); do
      sed -i "" "s|#include \"$h\"|#include \"$bucket/$h\"|g" "$f"
    done
  done
}

rewrite arch \
  minic.h multiboot.h irq.h lapic.h io_apic.h smp.h smp_detect.h \
  timer8254.h cpuid.h intr.h lowlevel.h segments.h context.h

rewrite core \
  thread.h threadimpl.h sched.h runq.h scheduler_inlined.h interrupt.h \
  javascheduler.h spinlock.h lock.h msg.h classes.h load.h execJAVA.h \
  portal.h portal_proto.h domain.h libcache.h atomic.h atomicfn.h \
  exception_handler.h code.h object.h syscalls.h malloc.h malloc_proto.h \
  memfs.h vmsupport.h zip.h minilzo.h lzoconf.h crc32.h bdt.h ekhz.h

rewrite debug \
  profile.h monitor.h serialdbg.h debug_reg.h bench.h symfind.h
```

Run: `bash rewrite_includes.sh`

Note: on GNU/Linux (e.g., CI), `sed -i ""` must become `sed -i`; on the macOS dev host the `""` form is correct.

- [ ] **Step 2: Fix the angle-bracket internal include in `syscalls.c`**

In `src/core/syscalls.c`, change:

```c
#include <syscalls.h>
```

to:

```c
#include "core/syscalls.h"
```

- [ ] **Step 3: Verify — every remaining bare quoted include must be a root header or a system header**

```bash
rg -o '#include "[^/"]+"' src --glob '*.c' --glob '*.h' --glob '*.S' | sort -u
```

Expected: only `all.h`, `config.h`, `types.h`, `misc.h`, `symbols.h`, `realmode.h`, and system headers (e.g. `string.h`, `stdio.h`, `lzo1x.h`) appear. Any moved header name listed here is a miss — fix it by running the matching `rewrite` line again.

- [ ] **Step 4: Remove the helper script**

```bash
rm rewrite_includes.sh
```

---

### Task 4: Update the Makefile

**Files:**
- Modify: `Makefile:1-52` (source lists + INCLUDES), `Makefile:108-110` (object flattening).

**Interfaces:**
- Consumes: Task 1 bucket locations.
- Produces: `make jxcore` resolves every source via `src/<bucket>/<file>.c` and links flat `.kernel/*.o` objects exactly as before.

- [ ] **Step 1: Rewrite `ESSENTIALSOURCES` (keep the `Memory/gc_*` continuation lines unchanged)**

Replace lines 1–6 with:

```makefile
ESSENTIALSOURCES = core/main.c core/libcache.c core/load.c core/thread.c core/interrupt.c \
                   core/domain.c core/misc.c Interface/zero.c core/vmsupport.c core/portal.c core/malloc.c \
				   core/classes.c core/zip.c core/execJAVA.c core/atomic.c \
                   core/exception_handler.c core/memfs.c core/atomicfn.c core/oneshot.c \
                   core/sched_global.c core/sched_local.c core/sched_local_rr.c core/sched_local_java.c core/sched_global_rr.c \
		   		   core/runq.c core/syscalls.c
```

- [ ] **Step 2: Rewrite `SUPPORTSOURCES`**

Replace line 12–13 with:

```makefile
SUPPORTSOURCES = debug/profile.c debug/thread_debug.c debug/thread_emulation.c debug/thread_profile.c \
                 debug/monitor.c Memory/gc_pa.c Memory/gc_pgc.c core/minilzo.c core/bdt.c core/crc32.c core/ekhz.c
```

- [ ] **Step 3: Rewrite `INCLUDES`**

Replace lines 37–41 with:

```makefile
INCLUDES = Headers/config.h Headers/core/thread.h Headers/core/load.h Interface/zero.h Headers/arch/lowlevel.h Headers/debug/profile.h \
           Memory/gc.h Memory/gc_memcpy.h Memory/gc_move.h Memory/gc_alloc.h Memory/gc_pa.h Memory/gc_pgc.h \
           Memory/gc_org.h Memory/gc_org_int.h Memory/gc_new.h Memory/gc_common.h Memory/gc_thread.h \
           Memory/gc_stack.h Memory/gc_impl.h Memory/gc_compacting.h \
           Headers/core/ekhz.h Headers/core/code.h Headers/core/libcache.h
```

- [ ] **Step 4: Rewrite `ESSENTIALCORESOURCES` and `CORESOURCES`**

Replace lines 43–44 with:

```makefile
ESSENTIALCORESOURCES =  arch/minic.c arch/multiboot.c arch/irq.c arch/lapic.c arch/io_apic.c arch/smp_detect.c arch/smp_activate.c arch/smp.c arch/timer8254.c arch/cpuid.c
CORESOURCES = $(SOURCES) $(ESSENTIALCORESOURCES) debug/serialdbg.c debug/symfind.c
```

- [ ] **Step 5: Extend the object-flattening substitutions**

Replace lines 108–110 with:

```makefile
COREOBJ2 = $(COREOBJ:.kernel/Memory/%=.kernel/%)
COREOBJ3 = $(COREOBJ2:.kernel/Assembly/%=.kernel/%)
COREOBJ4 = $(COREOBJ3:.kernel/Interface/%=.kernel/%)
COREOBJ5 = $(COREOBJ4:.kernel/core/%=.kernel/%)
COREOBJ6 = $(COREOBJ5:.kernel/arch/%=.kernel/%)
COREOBJ7 = $(COREOBJ6:.kernel/debug/%=.kernel/%)
COREBUILD = $(LD) $(CORELDOPTS) -T linker.ld -o jxcore $(COREOBJ7)
```

- [ ] **Step 6: Sanity-check the resolved file lists**

```bash
make -n jxcore 2>/dev/null | rg -o 'src/[a-z]+/[a-z0-9_]+\.c' | sort -u
```

Expected: every `.c` path carries an `arch/`, `core/`, or `debug/` prefix (no bare `src/x.c`), plus the untouched `Memory/` and `Interface/` entries.

---

### Task 5: Delete `.ll` artifacts and gitignore them

**Files:**
- Modify: `.gitignore`
- Delete: the 19 `src/*.ll` files

**Interfaces:**
- Consumes: nothing.
- Produces: `src/` free of accidental LLVM-IR artifacts; future stray `.ll` files are ignored by git.

- [ ] **Step 1: Delete the `.ll` files**

```bash
git rm src/*.ll
```

Expected: 19 files deleted.

- [ ] **Step 2: Append `*.ll` to `.gitignore`**

```bash
printf '*.ll\n' >> .gitignore
```

- [ ] **Step 3: Verify**

```bash
git status --short -- src | head -30
```

Expected: the only untracked/modified entries under `src/` are the renames from Tasks 1–3 and the `.ll` deletions.

---

### Task 6: Build until `make jxcore` succeeds

**Files:**
- Modify: any `.c`/`.h`/Makefile line that the compiler reports as a miss (expected: near-zero if Tasks 1–5 were complete).

**Interfaces:**
- Consumes: everything so far.
- Produces: a linking `jxcore` binary — the acceptance gate.

- [ ] **Step 1: Run the build**

```bash
make jxcore 2>&1 | tee /tmp/jcore_build.log
```

- [ ] **Step 2: Fix any compile errors**

Compile errors will be of the form `fatal error: 'thread.h' file not found` (an include miss from Task 3 — add the bucket prefix) or `No rule to make target 'core/xyz.c'` (a Makefile list miss — add the file to the right list). Fix, then re-run Step 1. Iterate until the link completes with `-o jxcore` and the binary exists:

```bash
ls -la jxcore
```

- [ ] **Step 3: Confirm the link included the flattened objects**

```bash
ls .kernel/*.o | wc -l
```

Expected: same count as a pre-refactor build (108 objects were present before this change).

- [ ] **Step 4: Fallback if the local toolchain cannot complete a full build**

Run the dry-run and confirm every `.c` maps to an object and the link command references every object:

```bash
make -n jxcore 2>&1 | rg -c '\.o$|\.c'   # inspect for obviously empty/missing entries
make -n jxcore 2>&1 | rg 'o jxcore'
```

If the machine lacks the i386 cross-link (`ld.lld -m elf_i386`), note it and rely on CI (`make` on ubuntu-latest) as the gate.

---

### Task 7: Final clean verification and single commit

**Files:**
- Modify: none (verification only) except the staged refactor.

**Interfaces:**
- Consumes: Task 6 success.
- Produces: the single atomic refactor commit.

- [ ] **Step 1: Clean rebuild from scratch**

```bash
make clean
make jxcore
```

Expected: compiles and links cleanly.

- [ ] **Step 2: Review the staged change set**

```bash
git add -A
git status
```

Expected: only the renames (`.c`/`.h` into buckets), `Makefile`, `.gitignore`, the `.ll` deletions, and no changes under `OS/`, `Compiler/`, `Zero/`, `AIZero/`, `test/`.

- [ ] **Step 3: Spot-check that no kernel logic changed**

```bash
git diff --numstat -- src | sort -k1,1n | tail -5
```

Expected: all renames show `0 0` (pure moves, no added/removed lines).

- [ ] **Step 4: Commit**

```bash
git commit -m "refactor: restructure src/ into arch, core, debug buckets"
```

- [ ] **Step 5: Verify the commit contents**

```bash
git show --stat HEAD | head -20
```
