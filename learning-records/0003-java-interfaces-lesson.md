# Learning Record 0003: Java Interfaces Lesson

## Context
User asked to learn about Java interfaces. This is directly relevant to their mission of understanding FlintJVM — the JVM must handle `invokeinterface` bytecode, interface class loading, and itable dispatch.

## Key Insights
- Interfaces compile to the same `.class` format as classes, distinguished only by the `ACC_INTERFACE (0x0200)` bit in `access_flags`.
- `invokeinterface` (opcode `0xB9`) requires itable dispatch — different from vtable dispatch used by `invokevirtual`.
- FlintJVM handles `invokeinterface` via `flint_execution.cpp:invokeInterface()` and `CONSTANT_InterfaceMethodref` (tag 11).
- The `UnsupportedOperationException` in the actual RPi test is NOT interface-related — it's from `invokedynamic` (Java 11 string concat).

## Related Files
- `addon/FlintJVM/Inc/flint_opcodes.h` — opcode definitions including `OP_INVOKEINTERFACE`
- `addon/FlintJVM/Inc/flint_const_pool.h` — `CONST_INTERFACE_METHOD = 11`
- `addon/FlintJVM/Inc/flint_class_loader.h` — `CLASS_INTERFACE = 0x0200`
- `addon/FlintJVM/Inc/flint_execution.h` — `invokeInterface()` method

## Next Steps
- Could cover `invokedynamic` and Java 11+ invokedynamic-based string concat next — this is the actual blocker.
- Or cover class loading / the constant pool in more depth.
