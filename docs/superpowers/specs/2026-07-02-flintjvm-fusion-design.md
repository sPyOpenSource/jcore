# Design Spec: FlintJVM Kernel Fusion

**Date**: 2026-07-02
**Status**: Approved
**Scope**: Minimalist Proof-of-Concept

## 1. Overview
The goal is to integrate the `FlintJVM` into the x86_64 kernel as a first-class subsystem. The integration follows a modular approach where the JVM core is maintained in its own directory but compiled and linked as part of the kernel image.

## 2. Architecture
The system consists of three primary layers:

### 2.1 Core VM
The bytecode interpreter and class loading logic from `FlintJVM/`. This layer remains logically isolated and is compiled with kernel flags (`-ffreestanding`, `-target x86_64-pc-none-elf`).

### 2.2 Native Layer
The bridge between Java native methods and C++ implementations, located in `FlintJVM/Native/`.

### 2.3 JVM Bridge (`src/jvm/`)
A new kernel component that implements the `flint_system_api`. It acts as the adapter between the VM's generic requirements and the kernel's specific implementations.

**Data Flow**:
`Java Code` $\rightarrow$ `Bytecode Interpreter` $\rightarrow$ `Native Method/System API` $\rightarrow$ `JVM Bridge` $\rightarrow$ `Kernel Subsystems (TaskManager, MemoryManager, etc.)`.

## 3. Detailed Integration

### 3.1 Threading (1:1 Mapping)
- **Creation**: Each `flint_java_thread` is mapped to a kernel `Task`.
- **Lifecycle**: The JVM Bridge wraps the JVM execution entry point and passes it to `TaskManager::AddTask`.
- **Scheduling**: Java threads are scheduled by the kernel's round-robin scheduler.
- **State**: The `CPUState` of Java threads is managed by the `TaskManager`.

### 3.2 Memory Management
- **Allocation**: The JVM's internal allocator is modified to use `MemoryManager::malloc` for heap requests.
- **GC**: The existing `FlintJVM` Garbage Collection logic is retained, operating on memory blocks provided by the `MemoryManager`.
- **Addressing**: The JVM operates in the kernel's 64-bit virtual address space.

### 3.3 System API (The Bridge)
The `src/jvm/` component implements the following minimal services:
- **Console**: `System.out.println` $\rightarrow$ `printf`.
- **Memory**: `malloc`/`free` $\rightarrow$ `MemoryManager`.
- **Sync**: `flint_mutex` $\rightarrow$ Kernel spinlocks/Interrupt disabling.
- **Bootstrapping**: `JVM::Initialize()` is called in `kernelMain` after `MemoryManager` and `TaskManager` setup.

## 4. Error Handling
- **JVM Panics**: Critical VM errors trigger a kernel panic with diagnostic output.
- **Java Exceptions**: Handled internally by the JVM core.

## 5. Success Criteria
- Successful boot of the kernel with the JVM initialized.
- Execution of a simple "Hello World" Java class.
- Verification that Java threads are scheduled as distinct kernel tasks.
