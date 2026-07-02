# NEON matmul for TinyStories-15M on BBAI

The user wants to run TinyStories-15M on the BBAI using NEON SIMD. The bottleneck is matrix-vector multiply (43M FMAs per generation for a 12-token sequence). NEON gives ~4× speedup over scalar for the hot loop (4-wide FMA via vmlaq_f32). With prefetching and INT8 quantization as stretch goals, expected throughput goes from ~12 tok/s (scalar) to ~45 tok/s (NEON + prefetch) to ~90 tok/s (+ INT8).

Practical steps: add `-mfpu=neon -mfloat-abi=hard` to Makefile, replace llama2.c's matmul() with the ~15-line NEON intrinsic version, link TinyStories-15M weights (60 MB) as a C array.

**Implication:** This is now a concrete implementation target. The user needs: (1) the RTOS boot + UART working (done), (2) llama2.c ported (lesson 6), (3) NEON flags added to Makefile, (4) TinyStories weights linked. The Makefile change alone is the next concrete action.
