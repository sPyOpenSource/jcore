#include "all.h"

struct multiboot_info boot_info;

extern void init_serial(int port);
extern void gic_init(void);
extern void arch_timer_init(void);
extern void ser_putc(int x, char c);

static void worker_thread(void *param)
{
    int i;
    (void)param;

    for (i = 0; i < 5; i++) {
        dprintf("worker: hello from jcore thread! (iteration %d)\n", i);
        {
            volatile int j;
            for (j = 0; j < 1000000; j++);
        }
        threadyield();
    }

    dprintf("worker: done, exiting\n");
    thread_exit();
}

void start_domain_zero(void *dummy)
{
    ThreadDesc *t;
    (void)dummy;

    dprintf("domain zero started\n");

    t = createThread(domainZero, worker_thread, (void *)0x1234, STATE_RUNNABLE, SCHED_CREATETHREAD_DEFAULT);
    setThreadName(t, "Worker", NULL);

    thread_exit();
}

void main(void)
{
    ThreadDesc *initial;

    init_serial(0);

    dprintf("\n=== jcore RTOS on ARM ===\n");
    dprintf("init serial: OK\n");
    
    // gic_init();
    // dprintf("init GIC: OK\n");
    
    init_domainsys();
    dprintf("init domains: OK\n");
    
    threads_init();
    dprintf("init threads: OK\n");
    
    // arch_timer_init();
    // dprintf("init timer: OK\n");


    initial = createThread(domainZero, start_domain_zero, (void *)0, STATE_RUNNABLE, SCHED_CREATETHREAD_DEFAULT);
    setThreadName(initial, "DomainZero:InitialThread", NULL);

    dprintf("starting scheduler...\n");
    llm();
    thread_exit();

    for (;;);
}

/* Entry point called by start.S */
void main(void) { jcore(); }
