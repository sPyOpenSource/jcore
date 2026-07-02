# Mission: Porting x86-32 Codebase to x86-64

## Goal
Successfully migrate a software codebase written for the x86 (32-bit) architecture to the x86-64 (64-bit) architecture.

## Context
The user has an existing codebase targeting x86-32 and needs to transition it to x86-64 to leverage larger address spaces, more registers, and modern hardware capabilities.

## Success Criteria
- The codebase compiles for x86-64.
- All core functionality is preserved.
- Performance is maintained or improved.
- Memory safety and alignment issues related to the architecture change are resolved.

## Key Challenges
- Pointer size increase (4 bytes -> 8 bytes).
- Changes in calling conventions (e.g., register-based argument passing).
- Alignment requirements for 64-bit data.
- x86-64 specific instruction set extensions (SSE/AVX).
- Handling of legacy 32-bit code/libraries.
