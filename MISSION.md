# Mission: Implement a JVM for Raspberry Pi

## The Goal
Implement a Java Virtual Machine (JVM) that runs on the Raspberry Pi, using the Circle bare-metal environment as the underlying operating system.

## Why This Matters
Building a JVM from scratch on bare metal requires mastering several deep layers of computing:
1. **Instruction Set Architecture (ISA)**: Mapping JVM bytecode to ARM instructions.
2. **Memory Management**: Implementing a heap and a Garbage Collector (GC) on raw RAM.
3. **Runtime Environment**: Managing threads, stacks, and the execution loop.
4. **Hardware Interaction**: Bridging the JVM's high-level abstractions to Raspberry Pi peripherals.

## Success Criteria
- [ ] A bootable RPi image containing the Circle environment and the JVM.
- [ ] Successful execution of a simple "Hello World" Java class file.
- [ ] Implementation of a subset of the JVM Specification (e.g., basic arithmetic, local variables, and method invocation).
- [ ] A working memory allocator/GC to handle Java objects.
