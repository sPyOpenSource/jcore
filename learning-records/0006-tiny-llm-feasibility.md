# Tiny LLM on baremetal ARM is feasible

The user asked "run tiny llm" after the ML lesson. Established: a 15M–42M parameter model fits in the BBAI's 1 GB DDR (60–168 MB in FP32). Karpathy's llama2.c (~700 lines of C) compiles for baremetal with minimal changes — replace fopen/fread with linked weight array, replace printf with uart_puts, remove malloc. The "link into kernel" approach avoids needing any storage driver. Expected performance on A15 at 1.5 GHz: ~10–15 tokens/second for a 15M model.

**Implication:** This is a concrete, low-risk stretch goal for the RTOS. Once the scheduler and domain protection are working, running an LLM inference as a task would be a compelling demo that exercises scheduler, memory management, and UART I/O in a realistic way.
