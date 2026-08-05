# Design: Informative panic context

Date: 2026-08-05
Status: Approved (design review)

## Goal

Make kernel panic output more informative. Today `sys_panic()` prints only
`PANIC` / `PANIC:` and the message — with no indication of which CPU, domain,
thread, or instruction was active when the crash happened. All of that context
is available at panic time but unused. This design adds a single context header
to the panic output, shared by both panic paths.

## Scope

- `src/debug/thread_debug.c` — new `print_panic_context()` implementation
  (file already in the Makefile's `SUPPORTSOURCES`, so no build-system change).
- `src/Headers/debug/monitor.h` — declaration (already included via `all.h`).
- `src/core/misc.c` — `sys_panic()` calls the helper; remove the redundant
  `printf("PANIC\n")` + `printf("PANIC:\n")` pair.
- `src/core/domain.c` — `domain_panic()` calls the helper.

No changes to the stack-trace walker, the monitor, or any panic call sites.

## Behavior

### New function

```c
void print_panic_context(char *prefix, ThreadDesc *thread, DomainDesc *domain);
```

Prints one context line for the given thread/domain to the current output
(same `printf` path as today: serial in KERNEL builds, pty otherwise). The
caller passes the thread and domain explicitly because `domain_panic`'s target
domain may differ from the current thread's.

Header fields:

```
PANIC t=12345ms cpu=0 dom=1(TestDomain) thr=3.7(Worker) state=RUNNABLE
      eip=0x0804c123 (core:findClass) caller=0x0804a0ff (core:loadClass)
```

- `t=` uptime ms: `rdtsc` + `CYCL2MILLIS` (same as the kernel branch of
  `clock_getTimeInMillis`). Source: `src/Headers/misc.h`.
- `cpu=` `get_processor_id()`.
- `dom=` `domain->id(domain->domainName)`.
- `thr=` `thread->domain->id.thread->id` plus `thread->name` when set.
- `state=` `get_state(thread)`.
- `eip=` `thread->context[PCB_EIP]` plus symbol via `print_eip_info`.
- `caller=` `getCaller(1)` plus symbol — the function that called panic, since
  the message often does not name the call site.

The message and the existing stack trace follow the header unchanged.

### Call sites

- `sys_panic()`: calls `print_panic_context("PANIC", curthr(), curdom())`
  after `CLI`, replacing the two redundant `PANIC` prints. All other behavior
  (in-panic guard, LOG_PRINTF handling, `clean_domainsys`, stack trace,
  monitor fallback, `exit(1)`) is unchanged.
- `domain_panic()`: calls
  `print_panic_context("DOMAIN PANIC", curthr(), domain)`, replacing its
  current `printf("DOMAIN PANIC ...")` block.

## Edge cases

- NULL thread or domain (early boot, before scheduler init): the helper skips
  the unavailable fields and prints only what is valid. It must not call
  `ASSERTTHREAD` (which can recursively panic).
- Interrupts are already disabled by `sys_panic` (`CLI`); `rdtsc` is safe under
  `CLI`.
- No allocations, no locks, no new dependencies introduced by the helper.
- Symbol lookup failures already degrade to `( ??? )` in `print_eip_info`; the
  header inherits that behavior.

## Testing

1. Build both targets (kernel and non-kernel) to confirm the helper compiles
   in each configuration.
2. Boot the kernel in QEMU and trigger a panic (existing early-boot panic or a
   temporarily injected `sys_panic("test")`), then verify the header prints
   correct CPU/domain/thread/state/EIP/caller fields and that symbols resolve
   in the serial output.
3. Verify the stack trace still follows the header and the redundant `PANIC`
   line is gone.
