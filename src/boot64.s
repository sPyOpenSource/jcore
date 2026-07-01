.section .text
.extern kernelMain
.extern callConstructors
.global long_mode_start

.code64
long_mode_start:
    mov $0x10, %ax
    mov %ax, %ss
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs

    mov $kernel_stack_top, %rsp

    movl %ebx, %edi
    movl %eax, %esi

    push %rsi
    push %rdi

    call callConstructors
    
    pop %rdi
    pop %rsi
    
    call kernelMain

_stop:
    cli
    hlt
    jmp _stop

.section .bss
.align 16
kernel_stack:
    .skip 2 * 1024 * 1024
kernel_stack_top:
