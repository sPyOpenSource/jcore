# 1 "zero/zero_FastMemory.S"
# 1 "/home/spy/OS/jcore//"
# 1 "<built-in>"
# 1 "<command-line>"
# 1 "zero/zero_FastMemory.S"

# 1 "./exception_handler.h" 1
# 3 "zero/zero_FastMemory.S" 2
# 28 "zero/zero_FastMemory.S"
.globl vm_get8
.globl vm_get32
.globl vm_getLittleEndian32

.align 8
vm_get8:


# 51 "zero/zero_FastMemory.S"
 cmpl 4(%eax),%esi
 jae mem_throw_memory_out_of_range_exception_get8

 movl 8(%eax),%eax
 movsbl 0(%eax,%esi,1), %eax



 ret

.align 8
vm_get32:


# 80 "zero/zero_FastMemory.S"
 movl 4(%eax), %edi
 shrl $2, %edi
 cmpl %edi, %esi
 jae mem_throw_memory_out_of_range_exception_get32

 movl 8(%eax),%eax
 movl 0(%eax,%esi,4), %eax



 ret

.align 8
vm_getLittleEndian32:


# 111 "zero/zero_FastMemory.S"
 movl 4(%eax), %edi
 addl $4, %edi
 cmpl %edi, %esi
 jae mem_throw_memory_out_of_range_exception_getLE32

 movl 8(%eax),%eax
 movl 0(%eax,%esi,1), %eax



 ret
.align 8
vm_setLittleEndian32:


# 141 "zero/zero_FastMemory.S"
 movl 4(%eax), %edi
 addl $4, %edi
 cmpl %edi, %esi
 jae mem_throw_memory_out_of_range_exception_setLE32

 movl 8(%eax),%eax
 movl 0(%eax,%esi,1), %eax



 ret
# 171 "zero/zero_FastMemory.S"
mem_throw_memory_out_of_range_exception_get8:

 pushl -4
 call exceptionHandler
mem_throw_memory_out_of_range_exception_get32:

 pushl -4
 call exceptionHandler
mem_throw_memory_out_of_range_exception_getLE32:

 pushl -4
 call exceptionHandler
mem_throw_memory_out_of_range_exception_setLE32:

 pushl -4
 call exceptionHandler
