# 1 "switch.S"
# 1 "/home/spy/OS/jcore//"
# 1 "<built-in>"
# 1 "<command-line>"
# 1 "switch.S"
# 1 "context.h" 1
# 2 "switch.S" 2
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
# 3 "switch.S" 2
# 13 "switch.S"
 .text
 .align 8
 .globl internal_switch_to
 .type internal_switch_to,@function
internal_switch_to:

 cli

 movl 4(%esp),%ecx
 movl 0(%ecx),%ecx

 movl 0(%ecx),%ecx
 movl 0(%esp), %eax
 movl %eax, 48(%ecx)
 movl %ebx, 32(%ecx)
 movl %esp, %eax
 addl $4, %eax
 movl %eax, 28(%ecx)
 movl %ebp, 24(%ecx)
 movl %esi, 20(%ecx)
 movl %edi, 16(%ecx)





 pushf
 popl 52(%ecx)
# 55 "switch.S"
 movl 8(%esp),%ecx




 movl 4(%esp),%edx
 movl %ecx,(%edx)




 movl 0(%ecx),%edx






 movl 28(%edx),%esp
 pushl 48(%edx)
 pushl 52(%edx)
 movl 16(%edx),%edi
 movl 20(%edx),%esi
 movl 24(%edx),%ebp
 movl 44(%edx),%eax
 movl 40(%edx),%ecx
 movl 32(%edx),%ebx
 movl 36(%edx),%edx

 popf

 ret
.globl internal_switch_to_end
internal_switch_to_end:
 .size internal_switch_to, internal_switch_to_end-internal_switch_to
 nop





 .text
 .align 8
 .globl destroy_switch_to
 .type destroy_switch_to,@function
destroy_switch_to:
# 112 "switch.S"
 movl 8(%esp),%ecx



 movl 4(%esp),%edx
 movl %ecx,(%edx)




 movl 0(%ecx),%edx





 movl 16(%edx),%edi
 movl 20(%edx),%esi
 movl 24(%edx),%ebp
 movl 44(%edx),%eax

 movl 28(%edx),%esp
 movl 48(%edx),%ebx
 pushl %ebx

 movl 16(%edx),%ebx
 pushl %ebx
 movl 20(%edx),%ebx
 pushl %ebx
 movl 24(%edx),%ebx
 pushl %ebx
 movl 44(%edx),%ebx
 pushl %ebx

 movl 40(%edx),%ebx
 pushl %ebx
 movl 32(%edx),%ebx
 pushl %ebx
 movl 52(%edx),%ebx
 movl 36(%edx),%edx
 pushl %edx
 pushl %ebx

 popf

 popl %edx
 popl %ebx
 popl %ecx

 popl %eax
 popl %ebp
 popl %esi
 popl %edi


 ret
.Ldestroy_switch_to_end:
 .size destroy_switch_to, .Ldestroy_switch_to_end-destroy_switch_to
