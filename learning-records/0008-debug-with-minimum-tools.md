# Learning Record 0008: Debugging with Minimum Tools (LEDs)

## The Problem
In baremetal development, you often encounter "silent crashes." If the UART is misconfigured or the CPU hits a HardFault before the console is initialized, you have zero visibility.

## The Solution: LED Signaling
LEDs provide a low-bandwidth, high-reliability channel to communicate system state.

### 1. The Checkpoint Pattern (Linear Progress)
Place LED toggles at critical milestones in the boot sequence:
- LED 1: Power on / Reset handler reached.
- LED 2: Clock initialization complete.
- LED 3: DDR initialized.
- LED 4: Kernel entry point reached.

**Insight:** If the board stops at LED 2, you know the crash happened during DDR initialization.

### 2. The Heartbeat (Liveness)
A slow, steady blink in a timer interrupt or the idle task.
- **Blinking:** System is alive, interrupts are firing, scheduler is running.
- **Solid/Off:** System is hung in a loop or a deadlock.

### 3. Error Codes (Binary/Pattern)
When you have multiple LEDs, you can signal specific error IDs:
- LED 1 ON, LED 2 OFF $\rightarrow$ Error 0x1 (Memory init fail)
- LED 1 OFF, LED 2 ON $\rightarrow$ Error 0x2 (Stack overflow)
- All ON $\rightarrow$ Panic/Critical Failure

### 4. The "Death Gasp"
A specific, high-frequency blink pattern triggered inside the `HardFault` or `Panic` handler. This distinguishes a "silent hang" (no LEDs) from a "loud crash" (rapid blinking).

## Summary for BBAI
On the BeagleBone AI, we map these signals to the user LEDs via the GPIO registers defined in the CSL.
