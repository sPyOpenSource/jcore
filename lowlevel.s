# 1 "lowlevel.S"
# 1 "/home/spy/OS/jcore//"
# 1 "<built-in>"
# 1 "<command-line>"
# 1 "lowlevel.S"
# 9 "lowlevel.S"
# 1 "context.h" 1
# 10 "lowlevel.S" 2
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
# 11 "lowlevel.S" 2


 .text
 .align 16
.globl activate_thread_desc
.globl get_thread_desc
.globl msetjmp
.globl mlongjmp
.globl send_msg1
.globl yield
.globl jxbcopy

.globl get_processor_id
.globl cas



    .align 4

.type jxbcopy,@function ;.globl jxbcopy ; jxbcopy:
 pushl %ebp
 movl %esp,%ebp
 pushl %edi
 pushl %esi
 movl 14(%ebp),%edx
 movl 12(%ebp),%edi
 movl 8(%ebp),%esi

0: cld
 movl %edx,%ecx
 sarl $2,%ecx
 js 1f
 rep
 movsl

 movl %edx,%ecx
 andl $3,%ecx
 rep
 movsb
1:
 popl %esi
 popl %edi
 popl %ebp
 ret



 .align 4

.type IN_jxbytecpy,@function ;.globl IN_jxbytecpy ; IN_jxbytecpy:
 pushl %ebp
 movl %esp,%ebp
        movl 8(%ebp),%esi
        movl 12(%ebp),%edi
        movl 16(%ebp),%ecx
        cld
        rep
 movsb
 movl %ebp,%esp
 popl %ebp
 ret
.LIN_jxbytecpy_end:
 .size IN_jxbytecpy, .LIN_jxbytecpy_end - IN_jxbytecpy


 .align 4

.type jxwordcpy,@function ;.globl jxwordcpy ; jxwordcpy:
 pushl %ebp
 movl %esp,%ebp
        movl 8(%ebp),%esi
        movl 12(%ebp),%edi
        movl 16(%ebp),%ecx
        cld
        rep
 movsl
 movl %ebp,%esp
 popl %ebp
 ret
.Ljxwordcpy_end:
 .size jxwordcpy, .Ljxwordcpy_end - jxwordcpy


 .align 4

.type jxmemset,@function ;.globl jxmemset ; jxmemset:
 push %ebp
 mov %esp,%ebp
 push %edi
 push %esi
 mov 0x8(%ebp),%edi
 mov 0x10(%ebp),%esi
 movzbl 0xc(%ebp),%eax
 cld
 cmp $0xb,%esi
 jbe 1f
 mov %eax,%edx
 shl $0x8,%edx
 or %eax,%edx
 mov %edi,%ecx
 mov %edx,%eax
 neg %ecx
 shl $0x10,%edx
 or %eax,%edx
 and $0x3,%ecx
 mov %edx,%eax
 sub %ecx,%esi
 repz stos %al,%es:(%edi)
 mov %esi,%ecx
 shr $0x2,%ecx
 repz stos %eax,%es:(%edi)
 and $0x3,%esi
1: mov %esi,%ecx
 repz stos %al,%es:(%edi)
 pop %esi
 mov 0x8(%ebp),%eax
 pop %edi
 pop %ebp
 ret


 .align 4

.type jxwmemset,@function ;.globl jxwmemset ; jxwmemset:
 push %ebp
 mov %esp, %ebp
 push %ecx
 push %edi
 movl 0x8(%ebp), %edi
 movl 0xc(%ebp), %eax
 movl 0xc(%ebp), %ecx
 andl $0xffff, %eax
 andl $0xffff, %ecx
 shll $16, %ecx
 orl %ecx, %eax
 movl 0x10(%ebp), %ecx

 cld
 shrl $1, %ecx
 jnc 1f
 stosb
1:
 shrl $1, %ecx
 jnc 2f
 stosw
2:
 rep stosl

 pop %edi
 pop %ecx
 pop %ebp
 ret




.type get_processor_id,@function ;.globl get_processor_id ; get_processor_id:
# 181 "lowlevel.S"
 movl $0,%eax
 ret

.Lget_processor_id_end:
 .size get_processor_id, .Lget_processor_id_end - get_processor_id
# 212 "lowlevel.S"
.type disable_cache,@function ;.globl disable_cache ; disable_cache:
        pushf
        pushl %eax
        cli
        movl %cr0, %eax

        orl $0x60000000, %eax
        movl %eax, %cr0
        wbinvd
 popl %eax
        popf
        ret

.Ldisable_cache: ; .size disable_cache, .Ldisable_cache-disable_cache


.type enable_cache,@function ;.globl enable_cache ; enable_cache:
        pushf
        pushl %eax
        cli
        wbinvd
 movl %cr0, %eax
        andl $0x9fffffff, %eax
        movl %eax, %cr0
        wbinvd
 popl %eax
        popf
        ret

.Lenable_cache_end: ; .size enable_cache, .Lenable_cache_end-enable_cache
