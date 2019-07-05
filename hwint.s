# 1 "hwint.S"
# 1 "/home/spy/OS/jcore//"
# 1 "<built-in>"
# 1 "<command-line>"
# 1 "hwint.S"
# 1 "context.h" 1
# 2 "hwint.S" 2
# 1 "thread.h" 1
# 10 "thread.h"
# 1 "misc.h" 1
# 11 "thread.h" 2
# 1 "load.h" 1
# 12 "thread.h" 2

# 1 "lock.h" 1
# 14 "thread.h" 2
# 1 "profile.h" 1



# 1 "config.h" 1
# 5 "profile.h" 2
# 1 "code.h" 1
# 6 "profile.h" 2
# 15 "thread.h" 2
# 1 "portal.h" 1
# 16 "thread.h" 2
# 1 "smp.h" 1
# 17 "thread.h" 2
# 1 "intr.h" 1
# 18 "thread.h" 2
# 3 "hwint.S" 2

# 1 "segments.h" 1
# 5 "hwint.S" 2


 .text

.globl hwint01
.globl hwint02
.globl hwint03
.globl hwint04
.globl hwint05
.globl hwint06
.globl hwint07
.globl hwint08
.globl hwint09
.globl hwint10
.globl hwint11
.globl hwint12
.globl hwint13
.globl hwint14
.globl hwint15
.global local_timer_int






 .align 4
 .type hwint,@function
 .globl hwint
hwint:
 pusha ; pushl %es ; pushl %fs ; pushl %gs ;
 call get_processor_id
 movl __current(,%eax,4), %ecx
 movl 0(%ecx), %ecx

 popl 0(%ecx)
 popl 4(%ecx)
 popl 8(%ecx)
 popl 16(%ecx)
 popl 20(%ecx)
 popl 24(%ecx)
 popl %edx
 popl 32(%ecx)
 popl 36(%ecx)
 popl 40(%ecx)
 popl 44(%ecx)

 popl %edx
 popl 48(%ecx)
 popl %ebx
 popl 52(%ecx)

 movl %esp, 28(%ecx)



 pushl %edx
 pushl %ecx
 pushl %eax

 call irq_first_level_handler
 jmp irq_panic

.Lhwint_end:
 .size hwint, .Lhwint_end - hwint

 .globl returnfromirq
returnfromirq:






 call get_processor_id
 movl __current(,%eax,4), %eax
 movl 0(%eax), %ecx

 movl 28(%ecx), %esp

 movl 52(%ecx), %eax
 pushl %eax

 movl $0x10, %eax
 pushl %eax

 movl 48(%ecx), %eax
 pushl %eax

 mov 0(%ecx), %gs
 mov 4(%ecx), %fs
 mov 8(%ecx), %es
 movl 16(%ecx), %edi
 movl 20(%ecx), %esi
 movl 24(%ecx), %ebp
 movl 36(%ecx), %edx
 movl 44(%ecx), %eax
 movl 32(%ecx), %ebx
 movl 40(%ecx), %ecx

 iret
.Lreturnfromirq_end:
 .size returnfromirq, .Lreturnfromirq_end - returnfromirq
# 119 "hwint.S"
        .align 4
hwint01:
        pushl $1 ; jmp hwint
        .align 4
hwint02:
        pushl $2 ; jmp hwint
        .align 4
hwint03:
        pushl $3 ; jmp hwint
        .align 4


hwint05:
        pushl $5 ; jmp hwint
        .align 4
hwint06:
        pushl $6 ; jmp hwint
        .align 4
hwint07:
        pusha ; pushl %es ; pushl %fs ; pushl %gs ;
 pushl $7
 call ackIRQ
 popl %eax
        popl %gs; popl %fs; popl %es; popa ;
 iret
        .align 4
hwint08:
        pushl $8 ; jmp hwint
        .align 4
hwint09:
        pushl $9 ; jmp hwint
        .align 4
hwint10:
        pushl $10 ; jmp hwint
        .align 4
hwint11:
        pushl $11 ; jmp hwint
        .align 4
hwint12:
        pushl $12 ; jmp hwint
        .align 4
hwint13:
        pushl $13 ; jmp hwint
        .align 4
hwint14:
        pushl $14 ; jmp hwint
 .align 4
hwint15:
        pushl $15 ; jmp hwint
# 197 "hwint.S"
 .align 8
 .globl activate_irq
activate_irq:
 movl 4(%esp),%ecx




 movl 0(%ecx), %edx
 movl 16(%edx), %edi
 movl 20(%edx), %esi
 movl 24(%edx), %ebp
 movl 44(%edx), %eax

 movl 28(%edx), %esp
 movl 48(%edx), %ebx
 pushl %ebx

 movl 16(%edx), %edi
 movl 20(%edx), %esi
 movl 24(%edx), %ebp
 movl 44(%edx), %eax
 movl 40(%edx), %ecx
 movl 32(%edx), %ebx
 movl 36(%edx), %edx

 ret
