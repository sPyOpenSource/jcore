#ifndef INTR_H
#define INTR_H

#ifndef ASSEMBLER
#include "types.h"

struct irqcontext {
    u4_t r0;
    u4_t r1;
    u4_t r2;
    u4_t r3;
    u4_t r4;
    u4_t r5;
    u4_t r6;
    u4_t r7;
    u4_t r8;
    u4_t r9;
    u4_t r10;
    u4_t r11;
    u4_t r12;
    u4_t sp;
    u4_t lr;
    u4_t pc;
    u4_t cpsr;
    u4_t irq_nr;
};

struct irqcontext_timer {
    u4_t r0;
    u4_t r1;
    u4_t r2;
    u4_t r3;
    u4_t r4;
    u4_t r5;
    u4_t r6;
    u4_t r7;
    u4_t r8;
    u4_t r9;
    u4_t r10;
    u4_t r11;
    u4_t r12;
    u4_t sp;
    u4_t lr;
    u4_t pc;
    u4_t cpsr;
};

#else

#define SAVE \
    stmfd sp!, {r0-r12, lr}  ;\

#define RESTORE \
    ldmfd sp!, {r0-r12, pc}  ;\

#endif

#endif
