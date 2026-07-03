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

extern void llm(void);

static void llama_thread(void *param)
{
    (void)param;
    llm();
    thread_exit();
}

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

static void fat32_test_thread(void *param)
{
    (void)param;
    struct mmc_block_dev ram_dev;
    struct fat32_fs fs;
    struct fat32_file fh;

    ram_blk_init_at(&ram_dev, 66000, (void *)0x45000000);

    if (fat32_mount(&ram_dev, &fs) != 0)
        dprintf("FAT32: mount FAILED\n");
    else if (fat32_open_root(&fs, "TEST.TXT", &fh) != 0)
        dprintf("FAT32: open FAILED\n");
    else {
        uint8_t buf[64];
        int n = fat32_read(&fh, buf, sizeof(buf), 0);
        buf[n < sizeof(buf) ? n : sizeof(buf) - 1] = '\0';
        dprintf("FAT32: %s\n", (char *)buf);
    }

    thread_exit();
}

extern struct vfs_ops fat32_vfs_ops;

static void vfs_test_thread(void *param)
{
    (void)param;
    struct mmc_block_dev ram_dev2;

    dprintf("VFS test: starting\n");

    ram_blk_init_at(&ram_dev2, 66000, (void *)0x45000000);
    vfs_init();
    dprintf("VFS: init OK\n");

    int rc = vfs_mount("/ram", &fat32_vfs_ops, &ram_dev2);
    dprintf("VFS: mount rc=%d\n", rc);

    int fd = vfs_open("/ram/test.txt", 0);
    dprintf("VFS: open fd=%d\n", fd);
    if (fd >= 0) {
        char buf[256];
        int n = vfs_read(fd, buf, sizeof(buf) - 1);
        dprintf("VFS: read %d bytes\n", n);
        if (n > 0) {
            buf[n] = '\0';
            dprintf("VFS: content: %s\n", buf);
        }
        vfs_close(fd);
    }
    dprintf("VFS test: done\n");
    thread_exit();
}

void start_domain_zero(void *dummy)
{
    ThreadDesc *t;
    (void)dummy;

    dprintf("domain zero started\n");

    t = createThread(domainZero, worker_thread, (void *)0x1234, STATE_RUNNABLE, SCHED_CREATETHREAD_DEFAULT);
    setThreadName(t, "Worker", NULL);

    t = createThread(domainZero, fat32_test_thread, (void *)0, STATE_RUNNABLE, SCHED_CREATETHREAD_DEFAULT);
    setThreadName(t, "FAT32_Test", NULL);

    t = createThread(domainZero, vfs_test_thread, (void *)0, STATE_RUNNABLE, SCHED_CREATETHREAD_DEFAULT);
    setThreadName(t, "VFS_Test", NULL);

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

    ThreadDesc *llm_t = createThread(domainZero, llama_thread, (void *)0, STATE_RUNNABLE, SCHED_CREATETHREAD_DEFAULT);
    setThreadName(llm_t, "LLM_Inference", NULL);

    dprintf("starting scheduler...\n");
    thread_exit();

    for (;;);
}
