#include "context.h"
#include "thread.h"

.text
.align 8

@ internal_switch_to(ThreadDesc **current, ThreadDesc *to)
@ Saves current thread context to PCB, restores to's context from PCB
.globl internal_switch_to
internal_switch_to:
    @ R0 = current (ThreadDesc**), R1 = to (ThreadDesc*)

    @ Load current thread PCB pointer
    ldr r2, [r0]                 @ R2 = *current = ThreadDesc*
    ldr r2, [r2, #THREAD_PPCB]   @ R2 = thread->contextPtr

    @ Save ARM callee-saved registers to PCB slots
    @ R9 -> context[PCB_GS]
    str r9,  [r2, #(PCB_GS * 4)]
    @ R10 -> context[PCB_FS]
    str r10, [r2, #(PCB_FS * 4)]
    @ R7 -> context[PCB_EDI]
    str r7,  [r2, #(PCB_EDI * 4)]
    @ R8 -> context[PCB_ESI]
    str r8,  [r2, #(PCB_ESI * 4)]
    @ R11 -> context[PCB_EBP]
    str r11, [r2, #(PCB_EBP * 4)]
    @ R4 -> context[PCB_EBX]
    str r4,  [r2, #(PCB_EBX * 4)]
    @ R5 -> context[PCB_EDX]
    str r5,  [r2, #(PCB_EDX * 4)]
    @ R6 -> context[PCB_ECX]
    str r6,  [r2, #(PCB_ECX * 4)]
    @ SP -> context[PCB_ESP]
    str sp,  [r2, #(PCB_ESP * 4)]
    @ LR -> context[PCB_EIP] (resume address)
    str lr,  [r2, #(PCB_EIP * 4)]

    @ Update current thread pointer
    str r1, [r0]                 @ *current = to

    @ Restore new thread context from PCB
    ldr r2, [r1, #THREAD_PPCB]   @ R2 = to->contextPtr

    @ Restore R0 (thread param) first - it's context[PCB_EAX]
    ldr r0,  [r2, #(PCB_EAX * 4)]

    @ Restore callee-saved registers from PCB slots
    ldr r9,  [r2, #(PCB_GS * 4)]
    ldr r10, [r2, #(PCB_FS * 4)]
    ldr r7,  [r2, #(PCB_EDI * 4)]
    ldr r8,  [r2, #(PCB_ESI * 4)]
    ldr r11, [r2, #(PCB_EBP * 4)]
    ldr r4,  [r2, #(PCB_EBX * 4)]
    ldr r5,  [r2, #(PCB_EDX * 4)]
    ldr r6,  [r2, #(PCB_ECX * 4)]

    @ Restore SP and resume address
    ldr sp,  [r2, #(PCB_ESP * 4)]
    ldr lr,  [r2, #(PCB_EIP * 4)]

    @ Jump to resume address (PCB_EIP = LR)
    bx lr

.globl internal_switch_to_end
internal_switch_to_end:


@ destroy_switch_to(ThreadDesc **current, ThreadDesc *to)
@ Restores to's context WITHOUT saving current thread's context.
@ Used when current thread is being destroyed.
.globl destroy_switch_to
destroy_switch_to:
    str r1, [r0]                 @ *current = to

    ldr r2, [r1, #THREAD_PPCB]   @ R2 = to->contextPtr

    ldr r0,  [r2, #(PCB_EAX * 4)]
    ldr r9,  [r2, #(PCB_GS * 4)]
    ldr r10, [r2, #(PCB_FS * 4)]
    ldr r7,  [r2, #(PCB_EDI * 4)]
    ldr r8,  [r2, #(PCB_ESI * 4)]
    ldr r11, [r2, #(PCB_EBP * 4)]
    ldr r4,  [r2, #(PCB_EBX * 4)]
    ldr r5,  [r2, #(PCB_EDX * 4)]
    ldr r6,  [r2, #(PCB_ECX * 4)]
    ldr sp,  [r2, #(PCB_ESP * 4)]
    ldr lr,  [r2, #(PCB_EIP * 4)]

    bx lr

.globl destroy_switch_to_end
destroy_switch_to_end:
