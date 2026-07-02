#ifndef CONTEXT_H
#define CONTEXT_H

/*
 * ARM context layout in the 14-slot ContextDesc array.
 * These offsets match what thread.c's pcb_init() and createThreadInMem() expect.
 *
 * | Index | x86 alias  | ARM reg  | Purpose                  |
 * |-------|------------|----------|--------------------------|
 * |  0    | PCB_GS     | R9       | Callee-saved             |
 * |  1    | PCB_FS     | R10      | Callee-saved             |
 * |  2    | PCB_ES     | (unused) |                          |
 * |  3    | PCB_DS     | (unused) |                          |
 * |  4    | PCB_EDI    | R7       | Callee-saved             |
 * |  5    | PCB_ESI    | R8       | Callee-saved             |
 * |  6    | PCB_EBP    | R11      | Frame pointer             |
 * |  7    | PCB_ESP    | SP       | Stack pointer             |
 * |  8    | PCB_EBX    | R4       | Callee-saved             |
 * |  9    | PCB_EDX    | R5       | Callee-saved             |
 * | 10    | PCB_ECX    | R6       | Callee-saved             |
 * | 11    | PCB_EAX    | R0       | Thread param / retval    |
 * | 12    | PCB_EIP    | LR       | Resume address            |
 * | 13    | PCB_EFLAGS | CPSR     | Program status register   |
 */

#define PCB_GS        0
#define PCB_FS        1
#define PCB_ES        2
#define PCB_DS        3
#define PCB_EDI       4
#define PCB_ESI       5
#define PCB_EBP       6
#define PCB_ESP       7
#define PCB_EBX       8
#define PCB_EDX       9
#define PCB_ECX       10
#define PCB_EAX       11
#define PCB_EIP       12
#define PCB_EFLAGS    13

/* ARM register aliases into the x86-named slots */
#define PCB_R0   PCB_EAX
#define PCB_R4   PCB_EBX
#define PCB_R5   PCB_EDX
#define PCB_R6   PCB_ECX
#define PCB_R7   PCB_EDI
#define PCB_R8   PCB_ESI
#define PCB_R9   PCB_GS
#define PCB_R10  PCB_FS
#define PCB_R11  PCB_EBP
#define PCB_SP   PCB_ESP
#define PCB_LR   PCB_EIP
#define PCB_CPSR PCB_EFLAGS

/* Segment register values for ARM (no-op, pcb_init writes these) */
#define KERNEL_DS  0
#define KERNEL_CS  0

#ifndef ASSEMBLER

typedef unsigned long ContextDesc[14];

#endif

#endif
