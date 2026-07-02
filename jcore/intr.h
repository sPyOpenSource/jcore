#ifndef INTR_H
#define INTR_H

#ifdef __arm__

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

#else /* ASSEMBLER */

#define SAVE \
	stmfd sp!, {r0-r12, lr} ;\

#define RESTORE \
	ldmfd sp!, {r0-r12, pc} ;\

#endif

#else /* x86 */

#ifndef ASSEMBLER

#include "types.h"

/*
 * This structure conforms to the stack frame format
 * that is built by the SAVE assembly macro.
 * 
 * !!! IF YOU MODIFY IT, PLEASE MODIFY SAVE/RESTORE AS WELL.
 */

struct irqcontext {
	u4_t gs, fs, es, ds;
	u4_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
	u4_t eip, cs, eflags;
#ifndef __KERNEL__
	u4_t esp0, ss0;
#endif
};

struct irqcontext_timer {
	u4_t gs, fs, es, ds;
	u4_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
	u4_t eip, cs, eflags;
};

/* When an user-level process gets an interrupt, the CPU pushes
 * the stack in the following order (Intel order):
 *     EFLAGS, CS, EIP
 * For kernel-level processes (privilege level 0), only these
 * three are pushed.
 */

/* Macro to save/restore processor state (context) on kernel stack */
#define SAVE \
	"cld\n"              \
	"pusha\n"            \
	"pushl %es\n"        \
	"pushl %fs\n"        \
	"pushl %gs\n"

#define RESTORE \
	"popl %gs\n"         \
	"popl %fs\n"         \
	"popl %es\n"         \
	"popa\n"

#endif

#endif /* __arm__ */

#endif				/* INTR_H */
