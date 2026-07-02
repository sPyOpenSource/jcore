#ifndef MISC_H
#define MISC_H

#ifndef ASSEMBLER


#ifdef DEBUG
#define ASSERT(x) if (!(x)) {printf("\"%s\", line %d: Assertion failed in file: %s.\n", __FILE__, __LINE__, #x); for(;;);}
#else
#define ASSERT(x)
#endif

void sys_panic(char *msg, ...);

#define MAX(a,b) (((a)<(b))?(b):(a))

#ifdef KERNEL
int dprintf(const char *fmt, ...);
#else
#define dprintf printf
#endif

static inline unsigned getEFlags()
{
    unsigned cpsr;
    asm volatile ("mrs %0, cpsr" : "=r" (cpsr));
    return cpsr;
}

#define setEFlags(cpsr) asm volatile("msr cpsr, %0" : : "r" (cpsr))

static inline int test_and_set_bit(int nr, volatile long *lock)
{
    unsigned long oldval;
    asm volatile (
        "   ldrex %0, [%2] \n"
        "   tst   %0, %3   \n"
        "   orreq %0, %0, %3 \n"
        "   strexeq %1, %0, [%2] \n"
        : "=&r" (oldval), "=&r" (nr)
        : "r" (lock), "r" (nr)
        : "cc", "memory"
    );
    return oldval;
}

static inline void atomic_inc(volatile int *v)
{
    int tmp;
    asm volatile (
        "1: ldrex %0, [%1] \n"
        "   add   %0, %0, #1 \n"
        "   strex %0, %0, [%1] \n"
        "   teq   %0, #0   \n"
        "   bne   1b       \n"
        : "=&r" (tmp)
        : "r" (v)
        : "cc", "memory"
    );
}

#define SLOW_DOWN_IO

static inline unsigned int inb(unsigned short int port)
{
    return 0;
}

static inline void outb(unsigned short int port, unsigned char val)
{
}

#define get_esp() \
    ({ \
        register unsigned int _temp__; \
        asm("mov %0, sp" : "=r" (_temp__)); \
        _temp__; \
    })

#define get_eflags() getEFlags()

#include "types.h"

char *strcat(char *dest, const char *src);
void sprintnum(char *s, u4_t u, int base);

void setTimer();

#define CYCL2NANOS(c)  ((u4_t)((((c)*(u8_t)1000))/(u8_t)CPU_MHZ))
#define CYCL2MICROS(c)  ((u4_t)((c)/(u8_t)CPU_MHZ))
#define CYCL2MILLIS(c)  ((u4_t)((c)/(u8_t)CPU_MHZ)/(u8_t)1000)

#endif

#endif
