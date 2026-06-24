# Resources: FlintJVM Integration Guide

## Primary Sources

### FlintJVM Documentation
- FlintJVM is a tiny (8,000 loc) JVM built in C++, not a JIT. It aims to be portable, fast, and deterministic.
- **Original Repository**: <https://github.com/espressif/FlintJVM>
- **Source Code**: Located in `addon/FlintJVM/`

### FlintJVM Architecture Overview
**Key Components of FlintJVM:**

1. **Core Engine** (`flint.cpp`, `flint_execution.cpp`) - The bytecode interpreter.
2. **Native Interface** (`addon/FlintJVM/Native/Inc/`) - Headers defining functions the engine calls (your bridge to hardware).
3. **Platform Library** (`addon/FlintJVM/Native/NativeSrc/`) - Native implementations (on ESP32, they interface directly with the ESP peripherals).
4. **Java API Library** (`addon/FlintJVM/SystemAPI/`) - Provides Java-level functionality.
5. **Core Datatype Objects** (`flint_java_object.cpp`, etc.) - Basic Java object implementations.

### Circle Integration Barriers
- FlintJVM was designed for ESP32, not RPi.
- FlintJVM is NOT a drop-in replacement for the Circle kernel.
- Flavors: GR (GNU) vs. ACR (Arm Compiler). The original uses GNU.

## Secondary Sources
- Minimal information online about porting FlintJVM to RPi.
- Some 'esp32-specific' implementations that assume ESP32 hardware - we need to adapt the **Native Interface** layer.

## Custom Resources

### flint_native_interface.pdf (Working Document)
This document is a walkthrough of FlintJVM's interface. It will be populated as we discover the function signatures that need to be implemented.

### Hardware Reference
- RPi MMIO registers for GPIO, UART, and Timer.
- Circle's peripheral abstraction layer (in `addon/circle/`).
