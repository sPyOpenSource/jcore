# Resources for VM Implementation

## Core Literature
- [Crafting Interpreters](https://craftinginterpreters.com/) by Robert Nystrom. The definitive guide for building an interpreter and a VM from scratch.
- [The Java Virtual Machine Specification](https://docs.oracle.com/javase/specs/jvms/se8/index.html). The official specification for the JVM.

## Case Studies
- **FlintJVM**: The primary case study for this course. Focus on `flint_execution.cpp` for the main loop and `flint_java_object.cpp` for memory representation.
- **Lua VM**: Known for being an incredibly efficient and compact register-based VM.

## Tools
- **QEMU**: Used for running the fused kernel and debugging the low-level interaction.
- **objdump / nm**: Essential for inspecting the final binary and verifying symbol placement.
