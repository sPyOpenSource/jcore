/********************************************************************************
 * Scheduler
 * Copyright 1998-2002 Michael Golm
 * Copyright 2001-2002 Meik Felser
 *******************************************************************************/

#ifndef JAVASCHEDULER

#ifndef RUNQ_H
#define RUNQ_H
#include "all.h"

struct runqueue_s {
	volatile DomainDesc *first;
	volatile DomainDesc *last;
};

/** ASSERTIONS ***/

#define ASSERTLOCKED ASSERTCLI

#ifdef DEBUG
#define ASSERTNIR(thread) if(check_not_in_runq(thread)==-1) sys_panic("NIR");
#else
#define ASSERTNIR(thread)
#endif


/** Prototypes ***/
static inline void runqueue_remove(ThreadDesc * thread);
static inline ThreadDesc *runqueue_removeFirstOrNULL(DomainDesc * currentDomain);
#if defined (DEBUG) || defined(CHECK_RUNNABLE_IN_RUNQ)
jboolean check_in_runq(ThreadDesc * thread);
void check_not_in_runq(ThreadDesc * thread);
inline void check_runq(void);
#endif


static inline void domainrunq_append(DomainDesc * domain);
static inline DomainDesc *domainrunq_remove();

static inline int threadrunq_append(DomainDesc * domain, ThreadDesc * thread);
static inline ThreadDesc *threadrunq_remove(DomainDesc * domain);

ThreadDesc *runqueue_removeFirstOrIdle(DomainDesc * currentDomain);
void runqueue_switchdomains(DomainDesc * from, DomainDesc * to);

void dump_runqOfDomain(DomainDesc * domain);

static inline void Sched_block(u4_t state);


#ifdef PRODUCTION
#define INLINE static inline
#include "runq_inline.c"
#endif


#endif				/* RUNQ_H */

#endif				/* JAVASCHEDULER */
