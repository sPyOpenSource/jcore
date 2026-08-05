# Informative Panic Context — Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add a single informative context header to every kernel panic so crashes identify the CPU, domain, thread, EIP symbol, and call site.

**Architecture:** One new function `print_panic_context(prefix, thread, domain)` lives in `src/debug/thread_debug.c` (already compiled by `SUPPORTSOURCES`). `sys_panic()` and `domain_panic()` call it with their current thread (and, for domain panics, the target domain). The helper prints the header, then the existing message + stack trace follow unchanged.

**Tech Stack:** C (gnu89, freestanding i386 kernel), clang `-target i386-pc-linux-gnu`, `ld.lld`, QEMU (`qemu-system-i386`) for behavioral verification. Custom kernel `printf` in `src/arch/minic.c` supports `%d %u %x %p %s %c`.

## Global Constraints

- Only touch: `src/debug/thread_debug.c`, `src/Headers/debug/monitor.h`, `src/core/misc.c`, `src/core/domain.c`, and temporarily `src/core/main.c` (test injection, reverted).
- No build-system changes: `thread_debug.c` is already in the Makefile's `SUPPORTSOURCES`; `monitor.h` is already included by `all.h`.
- No `ASSERTTHREAD`/`ASSERTDOMAIN` calls in the helper — panic code must never re-panic.
- The helper must tolerate NULL thread and NULL domain (early-boot panics). Do NOT call `curdom()` from `sys_panic` — the non-DEBUG inline `curdom()` dereferences `curthr()` (thread.h:254-257); pass `NULL` and let the helper derive the domain from the thread.
- Print to the current output via `printf` (serial in KERNEL builds, same path as today).
- Timestamp via `rdtsc` + `CYCL2MILLIS` (CPU_MHZ=500 is defined in settings.makefile; `CYCL2MILLIS` is a macro in `src/Headers/misc.h`).
- Build command (works, verified): `make jxcore`. Compile-flag baseline: `-DKERNEL -DDEBUG -DAPIC -DCPU_MHZ=500 -DHAVE_RDTSC`.
- In the DEBUG build, `curthr()` (thread.c:800) is NOT NULL-safe — a panic while `__current[cpu] == NULL` is a pre-existing crash, out of scope.

---

### Task 1: Declare `print_panic_context` in `src/Headers/debug/monitor.h`

**Files:**
- Modify: `src/Headers/debug/monitor.h`

**Interfaces:**
- Produces: `void print_panic_context(char *prefix, ThreadDesc *thread, DomainDesc *domain);` — consumed by Tasks 3 and 4.

- [ ] **Step 1: Add the prototype**

After the `printNStackTrace` declaration in `src/Headers/debug/monitor.h` (currently line 9-10), add:

```c
void print_panic_context(char *prefix, ThreadDesc * thread, DomainDesc * domain);
```

Resulting block:

```c
void checkStackTrace(ThreadDesc * thread, u4_t * base);
void printStackTrace(char *prefix, ThreadDesc * thread, u4_t * base);
void printNStackTrace(char *prefix, ThreadDesc * thread, u4_t * base,
		      int n);
void print_panic_context(char *prefix, ThreadDesc * thread, DomainDesc * domain);
```

(`ThreadDesc` is available via the existing `#include "core/thread.h"` at the top of the file; `DomainDesc` already appears in `printHeapUsage(DomainDesc *d)` below, so the include chain is sufficient.)

- [ ] **Step 2: Verify it compiles**

Run: `make .kernel/core/misc.o`
Expected: compiles with no errors (misc.o includes all.h → monitor.h).

- [ ] **Step 3: Commit**

```bash
git add src/Headers/debug/monitor.h
git commit -m "debug: declare print_panic_context prototype"
```

---

### Task 2: Implement `print_panic_context` in `src/debug/thread_debug.c`

**Files:**
- Modify: `src/debug/thread_debug.c` (append at end of file, after `get_state`)

**Interfaces:**
- Consumes: `get_state(ThreadDesc*)` (same file, line 271), `print_eip_info(char*)` (same file, line 177), `getCallerCallerIP()` (macro, `src/Headers/arch/lowlevel.h:63`), `get_processor_id()` (`arch/lapic.h`), `CYCL2MILLIS(c)` (`src/Headers/misc.h:142`), `PCB_EIP` (`src/Headers/arch/context.h`), `thread->context`, `thread->name[THREAD_NAME_MAXLEN]`, `thread->domain`, `domain->id`, `domain->domainName` (`src/Headers/core/thread.h`, `core/domain.h`).
- Produces: `void print_panic_context(char *prefix, ThreadDesc *thread, DomainDesc *domain)` — the header printer. Prints one line via `printf` and returns.

- [ ] **Step 1: Add the implementation**

Append to `src/debug/thread_debug.c`:

```c
void print_panic_context(char *prefix, ThreadDesc * thread, DomainDesc * domain)
{
	unsigned long long cycles;
	u4_t ms;

	asm volatile ("rdtsc":"=A" (cycles):);
	ms = CYCL2MILLIS(cycles);

	printf("%s t=%ums cpu=%d ", prefix, ms, get_processor_id());

	if (domain == NULL && thread != NULL)
		domain = thread->domain;

	if (domain != NULL)
		printf("dom=%d(%s) ", domain->id, domain->domainName);

	if (thread != NULL) {
		if (thread->domain != NULL)
			printf("thr=%d.%d", thread->domain->id, thread->id);
		else
			printf("thr=%d", thread->id);
		if (thread->name[0] != '\0')
			printf("(%s)", thread->name);
		printf(" state=%s eip=%p ", get_state(thread),
		       (void *) (u4_t) thread->context[PCB_EIP]);
		print_eip_info((char *) thread->context[PCB_EIP]);
		printf(" ");
	} else {
		printf("thr=(none) ");
	}

	printf("caller=");
	print_eip_info((char *) getCallerCallerIP());
	printf("\n");
}
```

Notes:
- `getCallerCallerIP()` unwinds the frame chain: from `print_panic_context` → caller (`sys_panic` or `domain_panic`) → return address in the function that called the panic. This is the same value the spec's `getCaller(1)` would yield, computed more cheaply.
- The `%p` specifier in the kernel printf prints `0x` + 8 hex digits (minic.c:738-747); passing a `void*` is consistent with existing usage in `monitor.c`.
- `thread->name` is a fixed `char[THREAD_NAME_MAXLEN]` array, never NULL, so guard on `name[0]`.
- No `ASSERTTHREAD`/`ASSERTDOMAIN` anywhere in this function.

- [ ] **Step 2: Verify it compiles**

Run: `make .kernel/debug/thread_debug.o`
Expected: compiles with no errors.

- [ ] **Step 3: Commit**

```bash
git add src/debug/thread_debug.c
git commit -m "debug: add print_panic_context helper"
```

---

### Task 3: Wire `print_panic_context` into `sys_panic`

**Files:**
- Modify: `src/core/misc.c:65-74` (the `in_panic = 1;` ... `printf("\n");` block inside `sys_panic`)

**Interfaces:**
- Consumes: `print_panic_context(char*, ThreadDesc*, DomainDesc*)` (Task 1/2), `curthr()` (`core/thread.h`).
- Produces: `sys_panic()` now prints the context header as its first line.

- [ ] **Step 1: Replace the redundant PANIC header block**

In `sys_panic` (src/core/misc.c), replace:

```c
	in_panic = 1;
	printf("PANIC\n");
#ifdef SMP
	printf("CPU%d ", get_processor_id());
#endif
	printf("PANIC:\n");
	printf("%s\n", msg);
```

with:

```c
	in_panic = 1;
	print_panic_context("PANIC", curthr(), NULL);
	printf("%s\n", msg);
```

- `curthr()` may be NULL (early boot, non-DEBUG inline path) — the helper handles it.
- Pass `NULL` for the domain on purpose: the helper derives it from `thread->domain`, avoiding the non-DEBUG `curdom()` NULL deref (see Global Constraints). The header still shows the current domain.
- The trailing `va_start`/`vprintf`/`va_end`/`printf("\n")`, `clean_domainsys`, stack trace, monitor fallback, and `exit(1)` stay untouched.

- [ ] **Step 2: Verify it compiles**

Run: `make .kernel/core/misc.o`
Expected: compiles with no errors.

- [ ] **Step 3: Full build check**

Run: `make jxcore`
Expected: link completes, `jxcore` produced.

- [ ] **Step 4: Commit**

```bash
git add src/core/misc.c
git commit -m "core: print panic context header in sys_panic"
```

---

### Task 4: Wire `print_panic_context` into `domain_panic`

**Files:**
- Modify: `src/core/domain.c:604-611` (the `printf("DOMAIN PANIC ...")` block inside `domain_panic`)

**Interfaces:**
- Consumes: `print_panic_context(char*, ThreadDesc*, DomainDesc*)` (Task 1/2), `curthr()` (`core/thread.h`), the `domain` parameter of `domain_panic(DomainDesc*, char*, ...)`.
- Produces: `domain_panic()` prints the context header naming the target domain.

- [ ] **Step 1: Replace the domain-panic header block**

In `domain_panic` (src/core/domain.c), replace:

```c
	printf("DOMAIN PANIC ");
	if (domain != NULL)
		printf("in domain %d", domain->id);
	printf("\n");
	va_start(args, msg);
```

with:

```c
	print_panic_context("DOMAIN PANIC", curthr(), domain);
	va_start(args, msg);
```

- The target `domain` is passed explicitly because it may differ from `curthr()->domain`. `domain` may be NULL (the helper's existing `if (domain != NULL)` guard covers it — same tolerance as the old code).
- `va_start(args, msg)` must immediately follow; the `vprintf`/`printf("\n")` and `printStackTrace("DOMAINPANIT ", ...)` lines stay untouched.

- [ ] **Step 2: Verify it compiles**

Run: `make .kernel/core/domain.o`
Expected: compiles with no errors.

- [ ] **Step 3: Full build check**

Run: `make jxcore`
Expected: link completes, `jxcore` produced.

- [ ] **Step 4: Commit**

```bash
git add src/core/domain.c
git commit -m "core: print panic context header in domain_panic"
```

---

### Task 5: Behavioral verification in QEMU (temporary injection, then revert)

Verifies the header actually prints correct CPU/domain/thread/state/EIP/caller and that the stack trace still follows. Uses a **temporary** `sys_panic` injected after `set_current(...)` in `main.c` (a point where the current thread is valid, so `curthr()` is safe even in the DEBUG build), then reverts the injection.

**Files:**
- Modify (temporarily): `src/core/main.c` (injection added after line 204, reverted before the final commit)
- Test data: `/tmp/jcore-empty.zip` (22-byte minimal empty zip, lets `zip_init` at main.c:139 pass so boot reaches the injection point)

**Interfaces:**
- Consumes: `sys_panic()` with the new header (Tasks 1-3).
- Produces: captured serial output proving the header.

- [ ] **Step 1: Create a minimal valid (empty) zip for the boot module**

```bash
printf '\x50\x4b\x05\x06' > /tmp/jcore-empty.zip
dd if=/dev/zero bs=1 count=18 >> /tmp/jcore-empty.zip
ls -l /tmp/jcore-empty.zip   # must be 22 bytes
```

`zip_init` (zip.c:98) reads the 22-byte End-Of-Central-Directory record; signature `PK\x05\x06`, `count=0`, `dir_size=0` — accepted without a module panic.

- [ ] **Step 2: Inject a test panic after the current thread exists**

In `src/core/main.c`, immediately after line 204 `set_current(createThread(...));` add:

```c
	sys_panic("test panic");
```

(The comment on line 205 is `#ifdef DEBUG` `check_current = 0;` — the injection goes between the `set_current` call and that block, so the current thread — the domainZero dummy thread, state RUNNABLE — is already valid.)

- [ ] **Step 3: Rebuild**

Run: `make jxcore`
Expected: link completes, `jxcore` produced.

- [ ] **Step 4: Boot in QEMU and capture serial output**

```bash
qemu-system-i386 -kernel jxcore -initrd /tmp/jcore-empty.zip -serial stdio -display none -no-reboot -d guest_errors 2>&1 | tee /tmp/panic.out | head -40
```

Expected: output ends with lines resembling:

```
PANIC t=0ms cpu=0 dom=0( ... ) thr=0.1 state=RUNNABLE eip=0x00000000 (null) caller=0x0040.... (core:main)
test panic
PANIC(0) ip=... sp=... stack=... (core:main)
...
```

Check ALL of:
1. Header is a single line starting `PANIC t=` with a numeric `cpu=0`.
2. `dom=` shows a numeric domain id (0 for domainZero) and a non-empty name.
3. `thr=` shows `0.<id>` with a state (RUNNABLE) — not `(none)`.
4. `eip=` prints an address plus a resolved symbol or `(null)`.
5. `caller=` resolves to `(core:main)` (the `findCoreSymbol` path) — proving symbol lookup works.
6. The message line `test panic` and the existing `PANIC(0) ip=...` stack-trace frames follow the header.
7. There is exactly ONE `PANIC` occurrence on the header line (the old double `PANIC`/`PANIC:` is gone).

- [ ] **Step 5: Revert the temporary injection**

Remove the `sys_panic("test panic");` line from `src/core/main.c`, then:

```bash
make jxcore
git status   # src/core/main.c must show no diff
```

Expected: build succeeds; `git status` shows `main.c` unchanged.

- [ ] **Step 6: Final commit of the verified state**

```bash
git add -A
git status   # only the 4 intended files should be staged
git commit -m "debug: improve panic output with context header"
```

---

## Self-Review

**Spec coverage:**
- Header line with `t/ cpu/ dom/ thr/ state/ eip/ caller/` → Task 2.
- `sys_panic` calls helper, removes double PANIC → Task 3.
- `domain_panic` calls helper with target domain → Task 4.
- NULL-safety (no assert, skip unavailable fields) → Task 2 implementation + Global Constraints.
- No build-system changes → Global Constraints (thread_debug.c already in SUPPORTSOURCES).
- Testing: compile both paths + QEMU panic observation → Tasks 1-5.

**Placeholder scan:** every code block is complete; no TBD/TODO; commands have expected output.

**Type consistency:** `print_panic_context(char*, ThreadDesc*, DomainDesc*)` is declared (Task 1), defined (Task 2), and called with `("PANIC", curthr(), NULL)` (Task 3) and `("DOMAIN PANIC", curthr(), domain)` (Task 4) — signatures match. `ms` is `u4_t`, printed with `%u`; `get_processor_id()` returns `int`, printed with `%d`; both match the kernel printf's arg types (long promotion on i386, minic.c:773/print_unsigned).

**Known deviation from spec:** the spec said `caller=` via `getCaller(1)`; the plan uses the equivalent `getCallerCallerIP()` macro, which is cheaper and needs no stack walk. Same resulting value, same output format.
