#ifndef MISC_H
#define MISC_H

#ifndef ASSEMBLER

#ifdef __arm__

#define ASSERT(x)
#define MAX(a,b) (((a)<(b))?(b):(a))

void sys_panic(char *msg, ...);

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

#define setEFlags(cpsr) asm volatile("msr cpsr, %0" : : "r" (cpsr) : "memory")

static inline int test_and_set_bit(int nr, volatile long *lock)
{
	unsigned long oldval, tmp;
	asm volatile (
		"1: ldrex %0, [%2] \n"
		"   tst   %0, %3   \n"
		"   bne   2f       \n"
		"   orr   %1, %0, %3 \n"
		"   strex %1, %1, [%2] \n"
		"   teq   %1, #0   \n"
		"   bne   1b       \n"
		"2:"
		: "=&r" (oldval), "=&r" (tmp)
		: "r" (lock), "r" (nr)
		: "cc", "memory"
	);
	return oldval & nr;
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

#else /* x86 */

#ifdef DEBUG
#define ASSERT(x) if (!(x)) {printf("\"%s\", line %d: Assertion failed in file: %s.\n", __FILE__, __LINE__, #x); asm("int $3");}
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
	unsigned eflags;
	asm volatile ("pushfl ; popl %0":"=r" (eflags));
	return eflags;
}

#define setEFlags(eflags) asm volatile("pushl %0 ; popfl" : : "r" (eflags))

static inline int test_and_set_bit(int nr, volatile long *lock)
{
	int oldbit;
	asm volatile ("lock;"
		      "tsl %2,%1;" "bbl %0,%0;":"=r" (oldbit), "=m"(*lock)
		      :"Ir"(nr));
	return oldbit;
}
static inline void atomic_inc(volatile int *v)
{
	asm volatile ("lock;" "incl %0;":"=m" (*v)
		      :"m"(*v));
}

#define get_cr4() \
    ({ \
	register unsigned int _temp__; \
	asm volatile("mov %%cr4, %0" : "=r" (_temp__)); \
	_temp__; \
    })
#define set_cr4(value) \
    ({ \
	register unsigned int _temp__ = (value); \
	asm volatile("mov %0, %%cr4" : : "r" (_temp__)); \
     })

#define set_idt(pseudo_desc) \
    ({ \
	asm volatile("lidt %0" : : "m" ((pseudo_desc)->limit)); \
    })

#define SLOW_DOWN_IO __asm__ __volatile__("outb %al,$0x80")

static inline unsigned int inb(unsigned short int port)
{
	unsigned char ret;
	asm volatile ("inb %1,%0": "=a" (ret): "d"(port));
	return ret;
}

static inline void outb(unsigned short int port, unsigned char val)
{
	asm volatile ("outb %0,%1"::"a" (val), "d"(port));
}

#define get_esp() \
    ({ \
	register unsigned int _temp__; \
	asm("movl %%esp, %0" : "=r" (_temp__)); \
	_temp__; \
    })

#define get_eflags() \
    ({ \
	register unsigned int _temp__; \
	asm volatile("pushfl; popl %0" : "=r" (_temp__)); \
	_temp__; \
    })

#endif /* __arm__ */

#include "types.h"

char *strcat(char *dest, const char *src);
void sprintnum(char *s, u4_t u, int base);

void setTimer();

#define CYCL2NANOS(c)  ((u4_t)((((c)*(u8_t)1000))/(u8_t)CPU_MHZ))
#define CYCL2MICROS(c)  ((u4_t)((c)/(u8_t)CPU_MHZ))
#define CYCL2MILLIS(c)  ((u4_t)((c)/(u8_t)CPU_MHZ)/(u8_t)1000)

#endif				/* ASSEMBLER */

#endif				/* MISC_H */
