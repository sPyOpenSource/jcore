.arm
.global _start
.global vector_table
.global irq_handler_asm
.global fiq_handler_asm

_start:
    ldr pc, _reset_vector
    ldr pc, _undefined_vector
    ldr pc, _svc_vector
    ldr pc, _prefetch_abort_vector
    ldr pc, _data_abort_vector
    ldr pc, _unused_vector
    ldr pc, _irq_vector
    ldr pc, _fiq_vector

_reset_vector:        .word reset_handler
_undefined_vector:    .word undefined_handler
_svc_vector:          .word svc_handler
_prefetch_abort_vector: .word prefetch_abort_handler
_data_abort_vector:   .word data_abort_handler
_unused_vector:       .word unused_handler
_irq_vector:          .word irq_handler_asm
_fiq_vector:          .word fiq_handler_asm

reset_handler:
    @ Set VBAR to our vector table
    ldr r0, =_start
    mcr p15, 0, r0, c12, c0, 0
    isb

    @ Enable FPU/NEON (CP10, CP11)
    mrc p15, 0, r0, c1, c0, 2
    orr r0, r0, #(0xF << 20)
    mcr p15, 0, r0, c1, c0, 2
    isb

    @ Enable FPU/NEON unit
    mov r0, #0x40000000
    vmsr fpexc, r0
    isb

    @ Enable unaligned memory access (SCTLR.U=1, SCTLR.A=0)
    mrc p15, 0, r0, c1, c0, 0
    bic r0, r0, #(1 << 1)     @ clear A (alignment check)
    orr r0, r0, #(1 << 22)    @ set U (unaligned access)
    mcr p15, 0, r0, c1, c0, 0
    isb

    @ Set SVC mode stack
    ldr sp, =stack_top_svc

    @ Set IRQ mode stack
    mrs r0, cpsr
    bic r0, r0, #0x1f
    orr r0, r0, #0x12          @ IRQ mode
    msr cpsr, r0
    ldr sp, =stack_top_irq
    isb

    @ Set FIQ mode stack
    mrs r0, cpsr
    bic r0, r0, #0x1f
    orr r0, r0, #0x11          @ FIQ mode
    msr cpsr, r0
    ldr sp, =stack_top_fiq
    isb

    @ Back to SVC mode
    mrs r0, cpsr
    bic r0, r0, #0x1f
    orr r0, r0, #0x13          @ SVC mode
    msr cpsr, r0

    ldr r3, =__bss_start
    ldr r4, =__bss_end
    cmp r3, r4
    beq bss_done
    mov r0, #0
bss_loop:
    str r0, [r3], #4
    cmp r3, r4
    blo bss_loop
bss_done:

    bl main

hang:
    b hang

undefined_handler:
    sub lr, lr, #4
    stmfd sp!, {r0-r12, lr}
    mov r0, sp
    bl arch_undefined_handler
    ldmfd sp!, {r0-r12, pc}^

svc_handler:
    stmfd sp!, {r0-r12, lr}
    mov r0, sp
    bl arch_svc_handler
    ldmfd sp!, {r0-r12, pc}^

prefetch_abort_handler:
    sub lr, lr, #4
    stmfd sp!, {r0-r12, lr}
    mov r0, sp
    bl arch_prefetch_abort_handler
    ldmfd sp!, {r0-r12, pc}^

data_abort_handler:
    sub lr, lr, #8
    stmfd sp!, {r0-r12, lr}
    mov r0, sp
    bl arch_data_abort_handler
    ldmfd sp!, {r0-r12, pc}^

unused_handler:
    stmfd sp!, {r0-r12, lr}
    mov r0, sp
    bl arch_unused_handler
    ldmfd sp!, {r0-r12, pc}^

irq_handler_asm:
    sub lr, lr, #4
    stmfd sp!, {r0-r12, lr}
    mrs r0, spsr
    stmfd sp!, {r0}
    mov r0, sp
    bl arch_irq_handler
    ldmfd sp!, {r0}
    msr spsr_cxsf, r0
    ldmfd sp!, {r0-r12, pc}^

fiq_handler_asm:
    sub lr, lr, #4
    stmfd sp!, {r0-r7, lr}
    mrs r0, spsr
    stmfd sp!, {r0}
    mov r0, sp
    bl arch_fiq_handler
    ldmfd sp!, {r0}
    msr spsr_cxsf, r0
    ldmfd sp!, {r0-r7, pc}^

.global get_processor_id
get_processor_id:
    mov r0, #0
    bx lr

.global irq_disable
irq_disable:
    cpsid i
    bx lr

.global irq_enable
irq_enable:
    cpsie i
    bx lr
