# 1 "schedSWITCH.S"
# 1 "/home/spy/OS/jcore//"
# 1 "<built-in>"
# 1 "<command-line>"
# 1 "schedSWITCH.S"
# 1 "thread.h" 1
# 10 "thread.h"
# 1 "misc.h" 1
# 11 "thread.h" 2
# 1 "load.h" 1
# 12 "thread.h" 2
# 1 "context.h" 1
# 13 "thread.h" 2
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
# 2 "schedSWITCH.S" 2
# 1 "segments.h" 1
# 3 "schedSWITCH.S" 2
# 200 "schedSWITCH.S"
 .text
 .align 8
 .globl call_java
 .type call_java,@function
call_java:

 movl 4(%esp),%ecx




 call save_context
# 220 "schedSWITCH.S"
 call get_processor_id
 movl 8(%esp),%ecx
 movl %ecx,__current(,%eax,4)



 movl 0(%ecx),%edx

 movl $0,%ebp
 movl 28(%edx),%esp
 pushl 48(%edx)

 ret
.Lcall_java_end:
 .size call_java, .Lcall_java_end-call_java
# 246 "schedSWITCH.S"
 .text
 .align 8
 .globl destroy_call_java
 .type destroy_call_java,@function
destroy_call_java:
# 259 "schedSWITCH.S"
 call get_processor_id
 movl 4(%esp),%ebx
 movl %ebx, __current(,%eax,4)



 movl 0(%ebx),%edx

 movl $0,%ebp
 movl 28(%edx),%esp
 pushl 48(%edx)

 pushl 52(%edx)
 popf



 ret
.Ldestroy_call_java_end:
 .size destroy_call_java, .Ldestroy_call_java_end-destroy_call_java
# 289 "schedSWITCH.S"
 .text
 .align 8
 .type return_form_java,@function
return_from_java:





 movl 8(%esp),%edx




 movl %eax, 44(%edx)


 cli
# 318 "schedSWITCH.S"
 call get_processor_id
 movl 4(%esp),%ecx
 movl %ecx,__current(,%eax,4)


 movl 8(%esp),%edx
# 333 "schedSWITCH.S"
 movl 28(%edx),%esp
 movl 16(%edx),%edi
 movl 20(%edx),%esi
 movl 24(%edx),%ebp
 movl 44(%edx),%eax

 movl 32(%edx),%ebx

 pushl 48(%edx)

 pushl 52(%edx)
 popf
# 362 "schedSWITCH.S"
 ret
.Lreturn_from_java_end:
 .size return_from_java, .Lreturn_from_java_end-return_from_java
# 382 "schedSWITCH.S"
 .global return_from_java0
 .global return_from_java1
 .global return_from_java2

 .type return_from_java0,@function
 .type return_from_java1,@function
 .type return_from_java2,@function

 .align 8
return_from_java2:
 popl %edx
return_from_java1:
 popl %edx
return_from_java0:
 jmp return_from_java
.global return_from_javaX_end
return_from_javaX_end:
 .size return_from_java0, return_from_javaX_end-return_from_java0
 .size return_from_java1, return_from_javaX_end-return_from_java1
 .size return_from_java2, return_from_javaX_end-return_from_java2
# 413 "schedSWITCH.S"
 .align 8
 .type save_context2current,@function
 .size save_context2current, .Lsave_context_end-save_context2current
.global save_context2current
save_context2current:
 call get_processor_id
 movl __current(,%eax,4),%edx
 movl 0(%edx),%ecx

 .type save_context,@function
save_context:


 movl 4(%esp),%eax
 movl %eax,48(%ecx)
 movl %ebx,32(%ecx)
 movl %esp,%eax
 addl $8, %eax
 movl %eax, 28(%ecx)
 movl %ebp,24(%ecx)
 movl %esi,20(%ecx)
 movl %edi,16(%ecx)






        pushf
 popl 52(%ecx)





 ret
.Lsave_context_end:
 .size save_context, .Lsave_context_end-save_context
