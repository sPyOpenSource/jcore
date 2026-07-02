# Domain/SVC misconception corrected

During the grilling session, the user proposed that putting the SVC table in domain 0 (MANAGER) would prevent task-code reads of it. This is backwards: MANAGER _bypasses_ all permission checks, making the table _more_ accessible, not less. The correct mechanism for protecting the SVC table from tasks is (a) not running user mode at all (AP bits don't help in SVC-only mode), or (b) using a different CLIENT domain for the table with appropriate AP bits if user mode is later added.

For a trust-based RTOS running everything in SVC mode, domain protection is between task memory regions, not between tasks and kernel — a buggy task can still corrupt any kernel data it can address.

**Implications:** The RTOS will be trust-based initially. Domain protection is useful for task-vs-task spatial isolation, but the kernel is always vulnerable to misbehaving tasks. Future session can cover adding user mode + system calls for true isolation.
