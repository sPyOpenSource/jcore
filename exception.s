# 1 "exception.S"
# 1 "/home/spy/OS/jcore//"
# 1 "<built-in>"
# 1 "<command-line>"
# 1 "exception.S"
# 9 "exception.S"
# 1 "context.h" 1
# 10 "exception.S" 2
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
# 11 "exception.S" 2


.globl divide_error
.globl debug_exception
.globl nmi
.globl breakpoint_exception
.globl overflow
.globl bounds_check
.globl inval_opcode
.globl copr_not_available
.globl double_fault
.globl copr_seg_overrun
.globl inval_tss
.globl segment_not_present
.globl stack_exception
.globl general_protection
.globl page_fault
.globl copr_error

.globl notDefined

.globl current_mthread

.text
# 62 "exception.S"
divide_error:
        pushl $0
        jmp exception

 .type debug_exception,@function
debug_exception:
 pusha ; pushl %es ; pushl %fs ; pushl %gs ;
        call debug_ex
 popl %gs; popl %fs; popl %es; popa ;
 iret
.Ldebug_exception_end:
 .size debug_exception, .Ldebug_exception_end-debug_exception

nmi:
        pushl $2
        jmp exception

 .type breakpoint_exception,@function
breakpoint_exception:
 pusha ; pushl %es ; pushl %fs ; pushl %gs ;
        call breakpoint_ex
 popl %gs; popl %fs; popl %es; popa ;
 iret
.Lbreakpoint_exception_end:
 .size breakpoint_exception, .Lbreakpoint_exception_end-breakpoint_exception

        .align 4
 .globl hwint04
hwint04:
 pusha ; pushl %es ; pushl %fs ; pushl %gs ;
        call ser_breakpoint_ex
 popl %gs; popl %fs; popl %es; popa ;
 iret

overflow:
        pushl $4
        jmp exception

bounds_check:
        pushl $5
        jmp exception

inval_opcode:
        pushl $6
        jmp exception

copr_not_available:
        pushl $7
        jmp exception


copr_seg_overrun:
        pushl $9
        jmp exception

double_fault:
        pushl $8
        jmp errexception

inval_tss:
        pushl $10
        jmp errexception

segment_not_present:
        pushl $11
        jmp errexception

stack_exception:
        pushl $12
        jmp errexception

general_protection:
        pushl $13
        jmp errexception

page_fault:
        pushl $14
        jmp errexception

copr_error:
        pushl $16
        jmp exception


        .align 4
 .type exception,@function
exception:
 pusha ; pushl %es ; pushl %fs ; pushl %gs ;


 call get_processor_id
 movl __current(,%eax,4),%ecx
 movl 0(%ecx),%ecx

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

 movl %esp,28(%ecx)






 pushl %eax
 pushl %edx
 call hw_exception
 iret
.Lexception_end:
 .size exception, .Lexception_end-exception

        .align 4
 .type errexception,@function
errexception:
 pusha ; pushl %es ; pushl %fs ; pushl %gs ;


 call get_processor_id
 movl __current(,%eax,4),%ecx
 movl 0(%ecx),%ecx

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
 popl %ebx
 popl 48(%ecx)
 popl %edi
 popl 52(%ecx)

 movl %esp,28(%ecx)


 pushl %eax
 pushl %ebx
 pushl %edx
 call hw_errexception
 iret
.Lerrexception_end:
 .size errexception, .Lerrexception_end-errexception

.align 4
exceptF:
except11:
except12:
except13:
except14:
except15:
except16:
except17:
except18:
except19:
except1A:
except1B:
except1C:
except1D:
except1E:
except1F:
notDefined:
        pushl $0xff
 jmp exception
