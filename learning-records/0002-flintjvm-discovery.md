# Learning Record 0002: Discovery of FlintJVM

## Context
While exploring the `circle` codebase for a JVM implementation, I discovered the `addon/FlintJVM` directory and the `sample/44-ESPJVM` sample.

## Key Insight
The "ESPJVM" is not a standalone JVM but an integration of the **FlintJVM** core library (`libwvm.a`) for the ESP32. This means a core, high-performance JVM implementation already exists within the Circle ecosystem.

## Decision
Instead of implementing a JVM from scratch, the project will pivot to **porting FlintJVM to the Raspberry Pi**. 

The focus shifts from implementing an interpreter and GC to implementing the **Native Interface** (HAL) that connects FlintJVM to the Raspberry Pi's hardware.

## Impact on Roadmap
- **Removed**: Bytecode interpreter implementation.
- **Removed**: GC from-scratch implementation.
- **Added**: Analysis of `addon/FlintJVM/Native/Inc`.
- **Added**: Implementation of RPi-specific native hooks for FlintJVM.
