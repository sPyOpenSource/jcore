#include "all.h"
#ifndef QEMU
#include "drivers/prcm.h"
#include "drivers/pinmux.h"
#include "drivers/mmc2.h"
#endif

#include "fs/vfs.h"
#include "fs/fat32.h"
#include "drivers/ram_blk.h"

struct multiboot_info boot_info;

extern void init_serial(int port);
extern void gic_init(void);
extern void arch_timer_init(void);
extern void irq_enable(void);

extern void llm_from_data(const char *model_data);

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

static char model_buffer[2 * 1024 * 1024];

extern struct vfs_ops fat32_vfs_ops;

static void vfs_llm_thread(void *param)
{
    (void)param;
    struct mmc_block_dev ram_dev;

    ram_blk_init_at(&ram_dev, 66000, (void *)0x45000000);
    vfs_init();

    if (vfs_mount("/sd", &fat32_vfs_ops, &ram_dev) != 0)
        dprintf("VFS: mount FAIL\n");
    else {
        int fd = vfs_open("/sd/MODEL.BIN", 0);
        if (fd < 0)
            dprintf("VFS: open FAIL\n");
        else {
            int n = vfs_read(fd, model_buffer, sizeof(model_buffer));
            vfs_close(fd);
            if (n > 0)
                llm_from_data(model_buffer);
        }
    }

    thread_exit();
}

void start_domain_zero(void *dummy)
{
    ThreadDesc *t;
    (void)dummy;

    dprintf("domain zero started\n");

    t = createThread(domainZero, worker_thread, (void *)0x1234, STATE_RUNNABLE, SCHED_CREATETHREAD_DEFAULT);
    setThreadName(t, "Worker", NULL);

    t = createThread(domainZero, vfs_llm_thread, (void *)0, STATE_RUNNABLE, SCHED_CREATETHREAD_DEFAULT);
    setThreadName(t, "VFS_LLM", NULL);

    thread_exit();
}

void main(void)
{
    ThreadDesc *initial;

#ifndef QEMU
    prcm_enable_mmc2();
    dprintf("PRCM: MMC2 clock enabled\n");

    pinmux_init_mmc2();
    dprintf("Pinmux: MMC2 pins configured\n");

    if (mmc2_init() == 0) {
        dprintf("MMC2: SD card initialized\n");
    } else {
        dprintf("MMC2: SD card init failed\n");
    }
#endif

    init_serial(0);

    dprintf("\n=== jcore RTOS on ARM ===\n");
    dprintf("init serial: OK\n");
    
    gic_init();
    dprintf("init GIC: OK\n");
    
    init_domainsys();
    dprintf("init domains: OK\n");
    
    threads_init();
    dprintf("init threads: OK\n");
    
    arch_timer_init();
    dprintf("init timer: OK\n");

    irq_enable();
    dprintf("IRQs enabled\n");

    initial = createThread(domainZero, start_domain_zero, (void *)0, STATE_RUNNABLE, SCHED_CREATETHREAD_DEFAULT);
    setThreadName(initial, "DomainZero:InitialThread", NULL);

    dprintf("starting scheduler...\n");
    thread_exit();

    for (;;);
}
