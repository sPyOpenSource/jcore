# FlintJVM Kernel Fusion Implementation Plan

> **For Hermes:** Use subagent-driven-development skill to implement this plan task-by-task.

**Goal:** Integrate FlintJVM into the x86_64 kernel as a minimalist proof-of-concept, allowing Java bytecode execution within kernel space.

**Architecture:** Modular integration. The JVM core and native layers are compiled as part of the kernel. A new `JVMBridge` component in `src/jvm/` maps JVM system requirements (memory, threads, I/O) to kernel subsystems.

**Tech Stack:** C++, x86_64, FlintJVM core, Kernel TaskManager, MemoryManager.

---

## Phase 1: Build System & Infrastructure

### Task 1: Create JVM Bridge directory and headers
**Objective:** Set up the folder structure and the bridge interface.

**Files:**
- Create: `src/jvm/jvm_bridge.h`
- Create: `src/jvm/jvm_bridge.cpp`

**Step 1: Create `src/jvm/jvm_bridge.h`**
```cpp
#ifndef __MYOS__JVM_BRIDGE_H
#define __MYOS__JVM_BRIDGE_H

#include <common/types.h>

namespace myos {
    namespace jvm {
        class JVMBridge {
        public:
            static void Initialize();
            static void* AllocateMemory(size_t size);
            static void FreeMemory(void* ptr);
            static void Println(const char* str);
        };
    }
}

#endif
```

**Step 2: Create `src/jvm/jvm_bridge.cpp`**
```cpp
#include <jvm/jvm_bridge.h>
#include <drivers/vga.h> // Or wherever printf is
#include <memorymanagement.h>

namespace myos {
    namespace jvm {
        void JVMBridge::Initialize() {
            // Basic init
        }

        void* JVMBridge::AllocateMemory(size_t size) {
            // To be implemented: call MemoryManager::malloc
            return 0;
        }

        void JVMBridge::FreeMemory(void* ptr) {
            // To be implemented
        }

        void JVMBridge::Println(const char* str) {
            // To be implemented: call printf
        }
    }
}
```

**Step 3: Commit**
```bash
git add src/jvm/jvm_bridge.h src/jvm/jvm_bridge.cpp
git commit -m "infra: add JVM bridge structure"
```

### Task 2: Update Makefile for FlintJVM Integration
**Objective:** Include FlintJVM source files in the kernel build process.

**Files:**
- Modify: `Makefile`

**Step 1: Add FlintJVM core and native files to `objects` list**
Add the following to the `objects` variable in `Makefile`:
```makefile
# Core VM
obj/flint.o \
obj/flint_array_object.o \
obj/flint_class_loader.o \
obj/flint_common.o \
obj/flint_const_pool.o \
obj/flint_debugger.o \
obj/flint_execution.o \
obj/flint_field_info.o \
obj/flint_fields_data.o \
obj/flint_java_class.o \
obj/flint_java_class_dict_node.o \
obj/flint_java_object.o \
obj/flint_java_string.o \
obj/flint_java_string_dict_node.o \
obj/flint_java_thread.o \
obj/flint_java_throwable.o \
obj/flint_method_info.o \
obj/flint_mutex.o \
obj/flint_utf8.o \
obj/flint_utf8_dict_node.o \
# Native Layer
obj/flint_native.o \
obj/flint_native_array.o \
obj/flint_native_character.o \
obj/flint_native_class.o \
obj/flint_native_constructor.o \
obj/flint_native_double.o \
obj/flint_native_float.o \
obj/flint_native_interface.o \
obj/flint_native_io_file.o \
obj/flint_native_io_file_descriptor.o \
obj/flint_native_io_file_input_stream.o \
obj/flint_native_io_file_output_stream.o \
obj/flint_native_math.o \
obj/flint_native_method.o \
obj/flint_native_object.o \
obj/flint_native_reflection.o \
obj/flint_native_string.o \
obj/flint_native_system.o \
obj/flint_native_thread.o \
obj/jvm/jvm_bridge.o \
```

**Step 2: Add build rules for FlintJVM files**
Add these rules to `Makefile`:
```makefile
obj/flint%.o: FlintJVM/%.cpp
	mkdir -p $(@D)
	$(CC) $(GCCPARAMS) -I FlintJVM/Inc -c -o $@ $<

obj/flint_native%.o: FlintJVM/Native/%.cpp
	mkdir -p $(@D)
	$(CC) $(GCCPARAMS) -I FlintJVM/Inc -I FlintJVM/Native/Inc -c -o $@ $<
```

**Step 3: Run `make clean && make` to verify compilation**
Expected: `mykernel.bin` produced without errors.

**Step 4: Commit**
```bash
git add Makefile
git commit -m "build: integrate FlintJVM sources into Makefile"
```

---

## Phase 2: JVM Bridge Implementation

### Task 3: Implement Memory Bridge
**Objective:** Map JVM memory requests to `MemoryManager`.

**Files:**
- Modify: `src/jvm/jvm_bridge.cpp`
- Modify: `FlintJVM/flint_common.cpp` (or wherever `malloc`/`free` are used)

**Step 1: Implement `AllocateMemory` and `FreeMemory` in `jvm_bridge.cpp`**
```cpp
void* JVMBridge::AllocateMemory(size_t size) {
    return MemoryManager::malloc(size);
}

void JVMBridge::FreeMemory(void* ptr) {
    // If MemoryManager has free(), call it. Otherwise, leave as no-op for POC.
}
```

**Step 2: Replace standard `malloc`/`free` in `FlintJVM` core with `JVMBridge` calls.**
Search for `malloc` and `free` in `FlintJVM/` and replace with `myos::jvm::JVMBridge::AllocateMemory` and `FreeMemory`.

**Step 3: Commit**
```bash
git add src/jvm/jvm_bridge.cpp FlintJVM/*.cpp FlintJVM/Native/*.cpp
git commit -m "feat: implement JVM memory bridge"
```

### Task 4: Implement Console Bridge
**Objective:** Map Java output to kernel `printf`.

**Files:**
- Modify: `src/jvm/jvm_bridge.cpp`
- Modify: `FlintJVM/flint_native_system.cpp` (or the system API implementation)

**Step 1: Implement `Println` in `jvm_bridge.cpp`**
```cpp
void JVMBridge::Println(const char* str) {
    printf(str);
    printf("\n");
}
```

**Step 2: Map JVM's system output call to `JVMBridge::Println`.**

**Step 3: Commit**
```bash
git add src/jvm/jvm_bridge.cpp FlintJVM/Native/flint_native_system.cpp
git commit -m "feat: implement JVM console bridge"
```

### Task 5: Implement Threading Bridge (1:1 Mapping)
**Objective:** Map Java thread creation to Kernel Tasks.

**Files:**
- Modify: `src/jvm/jvm_bridge.h`
- Modify: `src/jvm/jvm_bridge.cpp`
- Modify: `src/multitasking.h`
- Modify: `FlintJVM/flint_java_thread.cpp`

**Step 1: Add a wrapper function in `jvm_bridge.cpp` to serve as the Task entry point.**
```cpp
void JavaThreadEntry() {
    // Logic to start the specific Java thread execution
    // This will need to interact with the JVM's internal scheduler
}
```

**Step 2: Modify `flint_java_thread` creation logic to call `TaskManager::AddTask` with `JavaThreadEntry`.**

**Step 3: Ensure `CPUState` is correctly initialized for Java threads (matching the JVM's expected stack layout).**

**Step 4: Commit**
```bash
git add src/jvm/jvm_bridge.cpp src/multitasking.h FlintJVM/flint_java_thread.cpp
git commit -m "feat: implement 1:1 Java-to-Kernel thread mapping"
```

---

## Phase 3: Kernel Integration & Verification

### Task 6: JVM Initialization in Kernel
**Objective:** Boot the JVM during kernel startup.

**Files:**
- Modify: `src/kernel.cpp`
- Modify: `src/jvm/jvm_bridge.cpp`

**Step 1: Implement `JVMBridge::Initialize()`**
Call the JVM's internal initialization functions (from `flint.cpp`).

**Step 2: Call `JVMBridge::Initialize()` in `kernelMain`.**
Place it after `MemoryManager` and `TaskManager` are ready.

**Step 3: Commit**
```bash
git add src/kernel.cpp src/jvm/jvm_bridge.cpp
git commit -m "feat: initialize JVM during kernel boot"
```

### Task 7: Verify with "Hello World"
**Objective:** Execute a simple Java class and verify output.

**Step 1: Provide a simple `HelloWorld.class` file to the kernel (via a pre-loaded buffer or embedded array).**

**Step 2: Call the JVM's execution function to run `HelloWorld.main`.**

**Step 3: Run in QEMU and verify "Hello World" appears in the console.**

**Step 4: Final Commit**
```bash
git commit -m "test: verify JVM execution with Hello World"
```
