#ifndef ___SPINLOCK_H
#define ___SPINLOCK_H

typedef volatile unsigned int spinlock_t;

#define SPIN_LOCK_UNLOCKED (spinlock_t)1

#define spin_is_locked(x)	(*(volatile char *)(&x) <= 0)
#define spin_unlock_wait(x)	do { } while(spin_is_locked(x))

static inline int spin_trylock(spinlock_t * lock)
{
    unsigned int oldval;
    asm volatile (
        "   ldrexb %0, [%1] \n"
        "   cmp    %0, #0   \n"
        "   strexbeq %0, %2, [%1] \n"
        : "=&r" (oldval)
        : "r" (lock), "r" (0)
        : "cc", "memory"
    );
    return (int)oldval;
}

static inline void spin_lock(spinlock_t * lock)
{
    unsigned int tmp;
    asm volatile (
        "1: ldrexb %0, [%1] \n"
        "   cmp    %0, #0   \n"
        "   strexbeq %0, %2, [%1] \n"
        "   teq    %0, #0   \n"
        "   bne    1b       \n"
        "   dmb            \n"
        : "=&r" (tmp)
        : "r" (lock), "r" (0)
        : "cc", "memory"
    );
}

static inline void spin_unlock(spinlock_t * lock)
{
    asm volatile (
        "   dmb            \n"
        "   mov  %0, #1    \n"
        "   strb %0, [%1]  \n"
        : "=&r" (lock)
        : "r" (lock)
        : "memory"
    );
}

#endif
