# FlintJVM Project Overview

## What is FlintJVM?

**FlintJVM** is a tiny, portable JVM implementation written in C++ (8,000 lines of code). It's designed to be:

- **Small**: Minimal footprint (fits on embedded devices)
- **Deterministic**: Fixed-time garbage collection and predictable performance
- **Portable**: Multiple platform implementations (ESP32, Raspberry Pi, x86)

## Project Structure

### Core Files

#### `flint.cpp` / `flint.o`
- The main entry point and runtime engine
- Bootstrap JVM functionality and initialize core components

#### `flint_execution.cpp`
- The bytecode interpreter (JVM's "CPU")
- Implements most JVM opcodes for program execution

### Native Interface

#### `Native/Inc/` (Headers)
The FlintJVM Native Interface (JNI-like) – defined as pure virtual functions that each platform must implement.

#### `Native/flint_native.cpp` (Core Native Implementation)
- Contains the Native Bridge core logic
- Maps Java native methods to C++ function calls
- Used by all platforms (ESP32, RPi, etc.)

#### `Native/*` (Platform-Specific Native Files)
- Platform implementations of the Native Interface
- ESP32 specific: `addon/FlintJVM/Native/NativeSrc/` (for esp32)

### Java API (SystemAPI)

#### `SystemAPI/` 
- Java classes that provide core functionality (File I/O, System, etc.)

#### `src/`
- Application Java source (e.g., demo apps)

### Native Implementation (Platform-Specific HAL)

#### `libwvm.a`
- FlintJVM's static library (core engine + native bridge)

#### Platform-Specific Native Components
- Implementation of native methods for each platform
- For ESP32: `addon/FlintJVM/Native/NativeSrc/`
- For RPi: <strong>You need to implement this</strong>

### Usage

#### Building FlintJVM

To build FlintJVM, run:

```bash
make -C addon/FlintJVM
```

This produces `libwvm.a` in `addon/FlintJVM/`.

#### Example Usage (ESP32 Sample)

The `sample/44-ESPJVM/` contains an example of using FlintJVM in a Circle kernel for RPi/ESP32. The Makefile shows how to link FlintJVM's `libwvm.a` with the kernel.

### RPi Porting Strategy

1. **Implement the Native Interface** (`addon/FlintJVM/Native/Inc/`) 
   - Create a new class that implements FlintJVM's Native Interface
   - Implement functions like `native_uart_putc`, `native_system_get_time`, etc.

2. **Update Makefile**
   - Modify `addon/FlintJVM/Native/Makefile` to include your RPi native files
   - Build `libwvm.a` with RPi-specific implementations

3. **Update Sample Build**
   - Update `sample/44-ESPJVM/Makefile` to reference your new `libwvm.a`

### Current Status

The FlintJVM project is in a mature state with the following features:

- Interpreter that supports most Java opcodes
- Comprehensive Native Interface supporting platform-specific implementation
- Java API library for core functionality
- Portable across multiple platforms

To use FlintJVM on a new platform (e.g., RPi), you mainly need to implement the <strong>Native Interface layer</strong> for that platform.

### Limitations

- Not a Just-In-Time (JIT) compiler, purely interpreter-based
- Limited Java standard library support compared to full JDK
- Some advanced Java features may not be fully supported
