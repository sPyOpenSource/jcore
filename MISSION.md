# Mission: Preemptive RTOS on BeagleBone AI (Cortex-A15)

## Why

I'm building a preemptive RTOS (JX) for the BeagleBone AI's Cortex-A15. The RTOS is the core of a larger baremetal OS project. I need to understand how ARMv7-A exception handling, context switching, and memory protection work at the register level so I can write the scheduler, PendSV handler, and domain-based isolation from scratch.

## Success looks like

- Can explain how SysTick, PendSV, and SVC interact during a preemptive context switch
- Can write a PendSV handler that saves/restores r4–r11, SP, LR on ARMv7-A
- Can set up the MMU with domain-based protection: kernel domain 0 (MANAGER), task domains (CLIENT)
- Can build and run a minimal RTOS with two preempted tasks on real hardware
- Understand the DACR mechanism well enough to explain why the SVC table needs domain 0

## Constraints

- ARM32 mode (ARMv7-A), no hypervisor, no TrustZone
- Developing on macOS, cross-compiling with clang
- No external RTOS code — writing it from scratch for learning

## Out of scope

- SMP scheduling (using only one A15 core for now)
- DSP or M4 IPU programming
- Linux kernel internals
- Filesystem, networking, or any I/O beyond UART + GPIO
