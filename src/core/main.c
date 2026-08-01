#include "all.h"

extern DomainDesc *domainZero;
extern ClassDesc *java_lang_Object;

void dump_critical_vars(const char *label)
{
	printf("--- [%s] Critical Vars ---\n", label);
	printf("domainZero: %p\n", domainZero);
	printf("java_lang_Object: %p\n", java_lang_Object);
	
	// Dump a few bytes around java_lang_Object pointer in .bss
	u8_t *ptr = (u8_t *)&java_lang_Object;
	printf("Mem around java_lang_Object: ");
	for(int i = -4; i < 12; i++) {
		printf("%02x ", ptr[i]);
	}
	printf("\n---------------------------\n");
}

/*
 *
 * MAIN
 *
 */
void sigsegv_handler();
void start_domain_zero();

ObjectDesc *allocObject(ClassDesc * c);
ArrayDesc *allocArray(ClassDesc * elemClass, jint size);

void traceme()
{
	unsigned long int eflags;
	asm volatile ("pushfl;" "popl %0":"=r" (eflags));

	eflags |= 0x00000100;

	asm volatile ("pushl %0;" "popfl"::"r" (eflags));
}

void untraceme()
{
	unsigned long int eflags;
	asm volatile ("pushfl;" "popl %0":"=r" (eflags));

	eflags &= 0xfffffeff;

	asm volatile ("pushl %0;" "popfl"::"r" (eflags));
}

SharedLibDesc *zeroLib;

static void dummy_entry_point()
{
	sys_panic("dummy_entry_point SHOULD NOT BE CALLED");
}

//Proxy *domainZeroProxy;

/* This is the entry point for the initial JX managed thread */

char *zipstart;
jint ziplen;

#ifdef MICROBENCHMARKS
char *benchmem = NULL;
#endif				/* MICROBENCHMARKS */

#ifdef USE_EKHZ
CPUFrequency cpuFrequency;
#endif				/* USE_EKHZ */

#ifdef EVENT_LOG
void events_init()
{
	events = jxmalloc(sizeof(EventLog) * MAX_EVENTS);
}
#endif

int main(int argc, char *argv[])
{
	ThreadDesc *domainZero_thread;

#ifdef KERNEL
	struct multiboot_module *module;
#endif				/* KERNEL */

#ifdef USE_EKHZ
	getCPUFrequency(&cpuFrequency);
#endif				/* USE_EKHZ */

#ifndef KERNEL
	{
		sigset_t set, oldset;
		sigemptyset(&set);
		sigaddset(&set, SIGALRM);
		sigprocmask(SIG_BLOCK, &set, &oldset);
		printf("XXset sigmask %p - > %p\n", oldset, set);
	}
#endif

#ifndef KERNEL
	install_handler(SIGSEGV, sigsegv_handler);
	install_handler(SIGILL, sigsegv_handler);
	install_handler(SIGFPE, sigsegv_handler);

	jxmalloc_init();
#else				/* KERNEL */
	/* read zip from boot module */
	/* module = base_multiboot_find(ZIPFILE); */
		module = multiboot_get_module();

	if (module == NULL)
		sys_panic("Could not find boot module");

	printf("Module: mod_start=0x%x mod_end=0x%x size=%d\n",
	       module->mod_start, module->mod_end,
	       module->mod_end - module->mod_start);

	// Dump first 32 bytes of module data
	/*{
		unsigned char *p = (unsigned char *)module->mod_start;
		printf("Module[0:32]: ");
		int i;
		for (i = 0; i < 32; i++) printf("%02x ", p[i]);
		printf("\n");
	}
	// Dump last 64 bytes of module data
	{
		unsigned char *p = (unsigned char *)module->mod_start;
		int len = module->mod_end - module->mod_start;
		printf("Module[%d-32:%d]: ", len - 32, len);
		int i;
		for (i = len - 32; i < len; i++) printf("%02x ", p[i]);
		printf("\n");
	}*/

	zip_init(module->mod_start, module->mod_end - module->mod_start);
#endif				/* KERNEL */

#ifdef KERNEL
	//check_dirbuf("before pic_init_pmode");
	pic_init_pmode(); //check_dirbuf("after pic_init_pmode");
	init_irq_data(); //check_dirbuf("after init_irq_data");
	/*
	 * Serial line
	 */
	ser_enable_break(); //check_dirbuf("after ser_enable_break");

	printf("finished system init\n"); //check_dirbuf("after fin_sys_init");

#ifdef LOG_PRINTF
	init_log_space();
	printf2mem = 1;
#endif

#else
	init_irq_data();
#endif				/* KERNEL */
#ifdef EVENT_LOG
	events_init();
#endif

	init_domainsys();// check_dirbuf("after domainsys");
	//dump_critical_vars("After init_domainsys");

#ifdef KERNEL
#ifdef FRAMEBUFFER_EMULATION
	//init_realmode();
#endif
#endif

	/*
	 * Init preemption-aware atomic regions
	 */

#ifdef NOPREEMPT
	nopreempt_init();
#endif
	atomicfn_init(); //check_dirbuf("after atomicfn");
	//dump_critical_vars("After atomicfn_init");

	threads_init(); //check_dirbuf("after threads");
	//dump_critical_vars("After threads_init");
	portals_init();// check_dirbuf("after portals");
	//dump_critical_vars("After portals_init");

	//irq_disable(); /* don't need to disable interrupts, because there are none - timer not yet initialized */

#ifdef PROFILE
	profile_init();
#endif

	java_lang_Object = createObjectClassDesc();

	java_lang_Object_class = createObjectClass(java_lang_Object);

	createArrayObjectVTableProto(domainZero);
	//class_Array = createArrayObjectClassDesc(domainZero);
	//class_Array_class = createArrayObjectClass(domainZero, class_Array);
	/* init system */

	set_current(createThread(domainZero, dummy_entry_point /* dummy */ , (void *) -1, STATE_RUNNABLE, SCHED_CREATETHREAD_NORUNQ));	/* dummy thread */
#ifdef DEBUG
	check_current = 0;
#endif

#ifdef MICROBENCHMARKS
#define BMPRINT(txt) t1 = ((u8_t)bt.a) << 32 | (u8_t)bt.b;  t2 = ((u8_t)bt.c) << 32 | (u8_t)bt.d; t3 = t2 - t1; a = t3 >> 32; b = t3 & 0xffffffff; printf("%s 0x%lx%lx\n", txt, a, b);
#define FLUSHCACHE {int i; for(i = 0; i < totalmem; i++) {benchmem[i] = 1;}}
	{
		struct benchtime_s bt;
		u8_t t1, t2, t3;
		u4_t a, b;
		u4_t totalmem = 1024 * 1024;
		benchmem = jxmalloc(totalmem MEMTYPE_PROFILING);
		bench_empty(&bt);
		bench_empty(&bt);
		BMPRINT("empty");
		bench_store(&bt);
		bench_store(&bt);
		BMPRINT("store/hot");
		bench_store1(&bt);
		bench_store1(&bt);
		BMPRINT("store1/hot");
		FLUSHCACHE;
		bench_store1(&bt);
		BMPRINT("store1/cold");
		bench_load1(&bt);
		bench_load1(&bt);
		BMPRINT("load1/hot");
		FLUSHCACHE;
		bench_load1(&bt);
		BMPRINT("load1/cold");
		sys_panic("END OF BENCHMARK");
	}
#endif
	initPrimitiveClasses();
	domainZero_thread = createThread(domainZero, start_domain_zero, (void *) 0, STATE_RUNNABLE, SCHED_CREATETHREAD_DEFAULT);
	setThreadName(domainZero_thread, "DomainZero:InitialThread", NULL);
	thread_exit();

	/* not reached */
	return 0;		/* to satisfy compiler */
}
