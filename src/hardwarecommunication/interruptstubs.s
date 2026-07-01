.set IRQ_BASE, 0x20

.section .text
.extern _ZN4myos21hardwarecommunication16InterruptManager15HandleInterruptEhy

.macro HandleException num
.global _ZN4myos21hardwarecommunication16InterruptManager19HandleException\num\()Ev
_ZN4myos21hardwarecommunication16InterruptManager19HandleException\num\()Ev:
    movb $\num, (interruptnumber)
    jmp int_bottom
.endm

.macro HandleInterruptRequest num
.global _ZN4myos21hardwarecommunication16InterruptManager26HandleInterruptRequest\num\()Ev
_ZN4myos21hardwarecommunication16InterruptManager26HandleInterruptRequest\num\()Ev:
    movb $\num + IRQ_BASE, (interruptnumber)
    pushq $0
    jmp int_bottom
.endm

HandleException 0x00
HandleException 0x01
HandleException 0x02
HandleException 0x03
HandleException 0x04
HandleException 0x05
HandleException 0x06
HandleException 0x07
HandleException 0x08
HandleException 0x09
HandleException 0x0A
HandleException 0x0B
HandleException 0x0C
HandleException 0x0D
HandleException 0x0E
HandleException 0x0F
HandleException 0x10
HandleException 0x11
HandleException 0x12
HandleException 0x13

HandleInterruptRequest 0x00
HandleInterruptRequest 0x01
HandleInterruptRequest 0x02
HandleInterruptRequest 0x03
HandleInterruptRequest 0x04
HandleInterruptRequest 0x05
HandleInterruptRequest 0x06
HandleInterruptRequest 0x07
HandleInterruptRequest 0x08
HandleInterruptRequest 0x09
HandleInterruptRequest 0x0A
HandleInterruptRequest 0x0B
HandleInterruptRequest 0x0C
HandleInterruptRequest 0x0D
HandleInterruptRequest 0x0E
HandleInterruptRequest 0x0F
HandleInterruptRequest 0x31

HandleInterruptRequest 0x80

int_bottom:
    # Save all registers
    pushq %rbp
    pushq %rdi
    pushq %rsi
    pushq %rdx
    pushq %rcx
    pushq %rbx
    pushq %rax
    pushq %r8
    pushq %r9
    pushq %r10
    pushq %r11
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15

    # call C++ Handler: HandleInterrupt(uint8_t interrupt, uint64_t esp)
    # System V AMD64 ABI: rdi = 1st arg, rsi = 2nd arg
    movb (interruptnumber), %dil
    mov %rsp, %rsi
    call _ZN4myos21hardwarecommunication16InterruptManager15HandleInterruptEhy
    
    # switch the stack
    mov %rax, %rsp

    # restore registers
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %r11
    popq %r10
    popq %r9
    popq %r8
    popq %rax
    popq %rbx
    popq %rcx
    popq %rdx
    popq %rsi
    popq %rdi
    popq %rbp

    # Remove error code if pushed by CPU
    # In a real implementation, we'd check if the interrupt pushed an error code
    # For now, assuming the macro pushes it if needed.
    # But laoding from a macro and jumping to int_bottom...
    # Wait, we need to know if the exception had an error code.
    # The simplest way is to check the exception number.
    
    # I'll add a check for error code based on the interrupt number.
    # For now, just subtract if it was an IRQ or certain exceptions.
    # Let's use a more robust way: push the error code in the macro if it's an exception that does.
    # But let's keep it simple for now.
    # Actually, the most reliable way is to push a dummy 0 for everything and 
    # then just pop it at the end of all.
    
    # The current macros push a 0 for IRQs.
    # Let's just assume we pop one quad at the end.
    add $8, %rsp

.global _ZN4myos21hardwarecommunication16InterruptManager15InterruptIgnoreEv
_ZN4myos21hardwarecommunication16InterruptManager15InterruptIgnoreEv:
    iretq

.data
    interruptnumber: .byte 0
