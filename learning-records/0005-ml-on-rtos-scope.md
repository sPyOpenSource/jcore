# ML on RTOS: inference-only, training on server

The user asked "machine learning?" in the context of RTOS use cases. Established: only inference runs on an RTOS, not training. The BBAI has C66x DSPs and NEON SIMD for acceleration, but TI's ML stack requires Linux. For a baremetal RTOS, NEON-based inference on the A15 is the practical starting point. The killer app for RTOS ML is small, fast, deterministic models on streaming sensor data — not large models.

**Implication:** If the user wants ML on their RTOS, the next session should cover NEON intrinsics for matrix multiply. For now, it's a future possibility — the RTOS (scheduler, PendSV, domains) comes first.
