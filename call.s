# 1 "call.S"
# 1 "/home/spy/OS/jcore//"
# 1 "<built-in>"
# 1 "<command-line>"
# 1 "call.S"
# 1 "context.h" 1
# 2 "call.S" 2
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
# 3 "call.S" 2






 .text
 .align 16



 .type callnative_special,@function
 .globl callnative_special
callnative_special:
 pushl %ebp
 movl %esp,%ebp
 pushl %ebx;pushl %esi;pushl %edi
 movl 8(%ebp),%ecx
        movl 20(%ebp),%edx
        decl %edx
        js .L29
.L31:
        movl (%ecx,%edx,4),%eax
 pushl %eax
        decl %edx
        jns .L31
.L29:
 movl 12(%ebp),%eax
        pushl %eax
        movl 16(%ebp),%ecx
 call *%ecx







        movl 20(%ebp),%edx
 shll $2,%edx
 addl %edx,%esp
 addl $4,%esp
 popl %edi;popl %esi;popl %ebx
 leave

 ret
.global callnative_special_end
callnative_special_end:
 .size callnative_special, callnative_special_end-callnative_special
 nop



 .text
 .align 16



 .type callnative_special_portal,@function
 .globl callnative_special_portal
callnative_special_portal:
 pushl %ebp
 movl %esp,%ebp
 pushl %ebx;pushl %esi;pushl %edi

 sti




 movl 8(%ebp),%ecx
        movl 20(%ebp),%edx
        decl %edx
        js .L129
.L131:
        movl (%ecx,%edx,4),%eax
 pushl %eax
        decl %edx
        jns .L131
.L129:
 movl 12(%ebp),%eax
        pushl %eax
        movl 16(%ebp),%ecx
 call *%ecx

 cli
# 101 "call.S"
        movl 20(%ebp),%edx
 shll $2,%edx
 addl %edx,%esp
 addl $4,%esp
 popl %edi;popl %esi;popl %ebx
 leave

 ret
.global callnative_special_portal_end
callnative_special_portal_end:
 .size callnative_special_portal, callnative_special_portal_end-callnative_special_portal
 nop





 .type callnative_static,@function
 .globl callnative_static
callnative_static:
 pushl %ebp
 movl %esp,%ebp
 pushl %ebx;pushl %esi;pushl %edi
        movl 8(%ebp),%ecx
        movl 16(%ebp),%edx
        decl %edx
        js .L12
.L11:
        movl (%ecx,%edx,4),%eax
 pushl %eax
        decl %edx
        jns .L11
.L12:
        movl 12(%ebp),%ecx
        call *%ecx







        movl 16(%ebp),%edx
 shll $2,%edx
 addl %edx,%esp
 addl $4,%esp
 popl %edi;popl %esi;popl %ebx
 leave

 ret
.global callnative_static_end
callnative_static_end:
 .size callnative_static, callnative_static_end-callnative_static
 nop




 .type callnative_irq,@function
 .globl callnative_irq
callnative_irq:
 pushl %ebp
 movl %esp,%ebp
 pushl %ebx;pushl %esi;pushl %edi
 movl 8(%ebp),%eax
        pushl %eax
        movl 12(%ebp),%ecx
        call *%ecx







 addl $4,%esp
 popl %edi;popl %esi;popl %ebx
 leave

 ret
.Lcallnative_irq_end:
 .size callnative_irq, .Lcallnative_irq_end-callnative_irq




 .type callnative_handler,@function
 .globl callnative_handler
callnative_handler:
 movl 4(%esp),%ebp
 movl 12(%esp),%eax
 movl 8(%esp),%esp
 jmp *%eax
.Lcallnative_handler_end:
 .size callnative_handler, .Lcallnative_handler_end-callnative_handler


 .align 8
 .globl activate_oneshot
activate_oneshot:
 movl 4(%esp),%ecx




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
 movl 36(%edx),%edx
 pushl %edx

 popl %edx
 popl %ebx
 popl %ecx

 popl %eax
 popl %ebp
 popl %esi
 popl %edi

 ret
