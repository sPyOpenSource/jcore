#ifndef LOCK_H
#define LOCK_H

#ifndef ASSEMBLER

#include "types.h"

#ifdef DEBUG
struct DomainDesc_s;
void check_runnable(struct DomainDesc_s *domain);
void check_threadindomain(struct DomainDesc_s *domain);
#endif

#ifdef KERNEL
static inline unsigned int get_cpsr(void)
{
    unsigned int cpsr;
    asm volatile("mrs %0, cpsr" : "=r" (cpsr));
    return cpsr;
}

static inline void set_cpsr(unsigned int cpsr)
{
    asm volatile("msr cpsr, %0" : : "r" (cpsr));
}

#define SETEFLAGS(cpsr) set_cpsr(cpsr)

#ifdef CHECK_RUNNABLE_IN_RUNQ
#define DISABLE_IRQ    { volatile u4_t oldflags = get_cpsr(); asm volatile("cpsid i"); if ((oldflags&0x80)==0) foreachDomainRUNQ(check_runnable);
#define RESTORE_IRQ    if ((oldflags&0x80)==0) foreachDomainRUNQ(check_runnable); foreachDomainRUNQ(check_threadindomain); SETEFLAGS(oldflags); }
#else
#define DISABLE_IRQ    { volatile u4_t oldflags = get_cpsr(); asm volatile("cpsid i");
#define RESTORE_IRQ    SETEFLAGS(oldflags); }
#endif

#define ASSERTCLI ASSERT((get_cpsr() & 0x80) == 0x80)
#define ASSERTSTI ASSERT((get_cpsr() & 0x80) == 0)
#define CLI  asm volatile("cpsid i");

#else
#include <signal.h>
#ifdef CHECK_RUNNABLE_IN_RUNQ
#define DISABLE_IRQ   {   sigset_t set, oldset; disable_irq(&set, &oldset);  if (!sigismember(&oldset,SIGALRM)) foreachDomainRUNQ(check_runnable);
#define RESTORE_IRQ     if (!sigismember(&oldset,SIGALRM)) foreachDomainRUNQ(check_runnable); restore_irq(&oldset);}
#else
#define DISABLE_IRQ   {   sigset_t set, oldset; disable_irq(&set, &oldset);
#define RESTORE_IRQ     restore_irq(&oldset);}
#endif
#define ASSERTCLI {sigset_t set, oldset; sigemptyset(&set); sigprocmask(SIG_BLOCK, &set, &oldset); if (!sigismember(&oldset,SIGALRM)) sys_panic("ALARM NOT BLOCKED"); }
#define CLI  {sigset_t set; disable_irq(&set, (void*)0); }
void restore_irq(const sigset_t * set);
#endif

#ifndef DEBUG
#undef ASSERTCLI
#define ASSERTCLI
#undef ASSERTSTI
#define ASSERTSTI
#endif

#endif

#endif
