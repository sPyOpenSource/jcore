#ifndef ___SPINLOCK_H
#define ___SPINLOCK_H

#ifdef __arm__

typedef volatile unsigned int spinlock_t;

#define SPIN_LOCK_UNLOCKED (spinlock_t)1

#define spin_is_locked(x)	(*(volatile char *)(&x) <= 0)
#define spin_unlock_wait(x)	do { } while(spin_is_locked(x))

static inline int spin_trylock(spinlock_t * lock)
{
	unsigned long oldval;
	asm volatile (
		"   ldrexb %0, [%1] \n"
		"   cmp    %0, #1   \n"
		"   strexbeq %0, %2, [%1] \n"
		: "=&r" (oldval)
		: "r" (lock), "r" (0)
		: "cc", "memory"
	);
	return oldval == 1;
}

static inline void spin_lock(spinlock_t * lock)
{
	unsigned long tmp;
	asm volatile (
		"1: ldrexb %0, [%1] \n"
		"   cmp    %0, #1   \n"
		"   wfene            \n"
		"   strexbeq %0, %2, [%1] \n"
		"   teq    %0, #0   \n"
		"   bne    1b       \n"
		"   dmb             \n"
		: "=&r" (tmp)
		: "r" (lock), "r" (0)
		: "cc", "memory"
	);
}

static inline void spin_unlock(spinlock_t * lock)
{
	unsigned long tmp;
	asm volatile (
		"   dmb             \n"
		"   mov     %0, #1  \n"
		"   strb    %0, [%1] \n"
		: "=&r" (tmp)
		: "r" (lock)
		: "memory"
	);
}

static inline void spin_lock1(spinlock_t * lock)
{
	spin_lock(lock);
}

static inline void spin_unlock1(spinlock_t * lock)
{
	spin_unlock(lock);
}

#else /* x86 */

typedef volatile unsigned int spinlock_t;

#define SPIN_LOCK_UNLOCKED (spinlock_t)1

#define spin_is_locked(x)	(*(volatile char *)(&x) <= 0)
#define spin_unlock_wait(x)	do { } while(spin_is_locked(x))

static inline int spin_trylock(spinlock_t * lock)
{
	char oldval;
	__asm__ __volatile__("xchgb %b0,%1":"=q"(oldval), "=m"(*lock)
			     :"0"(0));
	return oldval > 0;
}

static inline void spin_lock(spinlock_t * lock)
{
	__asm__ __volatile__("1: " "  lock ; decb %0;" "  js 2f;"	/* already locked */
			     "  jmp 3f;"	/* lock aquired */
			     "2: " "  cmpb $0,%0;"	/* while locked */
			     "  rep;nop;"	/* do nothing */
			     "  jle 2b;"	/*  loop */
			     "  jmp 1b;"	/* unlocked -> try again */
			     "3:":"=m"(*lock));
}

static inline void spin_unlock(spinlock_t * lock)
{
	__asm__ __volatile__("movb $1,%0":"=m"(*lock));
}

static inline void spin_lock1(spinlock_t * lock)
{
	__asm__ __volatile__("1: 	lock ; btsl $0, %0"
			     "	jnc 3f"
			     "2:	testb $1, %0"
			     "	jne 2b" "	jmp 1b" "3:":"=m"(*lock));
}

static inline void spin_unlock1(spinlock_t * lock)
{
	__asm__ __volatile__("lock; btrl $0, %0":"=m"(*lock));
}

#endif /* __arm__ */

#endif
