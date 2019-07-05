# 1 "timer.S"
# 1 "/home/spy/OS/jcore//"
# 1 "<built-in>"
# 1 "<command-line>"
# 1 "timer.S"
# 11 "timer.S"
# 1 "config.h" 1
# 12 "timer.S" 2
# 1 "context.h" 1
# 13 "timer.S" 2
# 1 "thread.h" 1
# 10 "thread.h"
# 1 "misc.h" 1
# 11 "thread.h" 2
# 1 "load.h" 1
# 12 "thread.h" 2

# 1 "lock.h" 1
# 14 "thread.h" 2
# 1 "profile.h" 1




# 1 "code.h" 1
# 6 "profile.h" 2
# 15 "thread.h" 2
# 1 "portal.h" 1
# 16 "thread.h" 2
# 1 "smp.h" 1
# 17 "thread.h" 2
# 1 "intr.h" 1
# 18 "thread.h" 2
# 14 "timer.S" 2



.globl hwint00
# 52 "timer.S"
  .text

        .align 4
 .type hwint00,@function

hwint00:
# 66 "timer.S"
 iret
.Lhwint00_end:
 .size hwint00, .Lhwint00_end-hwint00
# 206 "timer.S"
 .text
 .align 8
 .globl irq_destroy_switch_to
 .type irq_destroy_switch_to,@function
irq_destroy_switch_to:



 movl 8(%esp),%ecx



 movl 4(%esp),%edx
 movl %ecx,(%edx)




 movl 0(%ecx),%edx

 movl 40(%edx),%ecx

 mov 0(%edx),%gs
 mov 4(%edx),%fs

 movl 16(%edx),%edi
 movl 20(%edx),%esi
 movl 24(%edx),%ebp
 movl 28(%edx),%esp


 movl 52(%edx),%ebx
 pushl %ebx

 movl $0x10, %ebx
 pushl %ebx

 movl 48(%edx),%ebx
 pushl %ebx

 movl 44(%edx),%ebx
 pushl %ebx
 movl 32(%edx),%ebx
 pushl %ebx
 movl 36(%edx),%edx
 pushl %edx


 popl %edx
 popl %ebx
 popl %eax

 iret
