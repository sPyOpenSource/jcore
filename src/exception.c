#include "all.h"

extern void gic_end_interrupt(unsigned int id);

static const char *exc_names[] = {
    "Reset", "Undefined Instruction", "Supervisor Call",
    "Prefetch Abort", "Data Abort", "Unused", "IRQ", "FIQ"
};

void arch_undefined_handler(unsigned int *frame)
{
    dprintf("EXCEPTION: %s\n", exc_names[1]);
    dprintf("Frame at %p: ", (void*)frame);
    for (int i = 0; i < 16; i++)
        dprintf("r%d=%08x ", i, frame[i]);
    dprintf("\n");
    sys_panic("Undefined instruction exception");
}

void arch_svc_handler(unsigned int *frame)
{
    (void)frame;
}

void arch_prefetch_abort_handler(unsigned int *frame)
{
    unsigned int far;
    asm volatile("mrc p15, 0, %0, c6, c0, 2" : "=r" (far));
    dprintf("EXCEPTION: %s at 0x%08x\n", exc_names[3], far);
    dprintf("Frame at %p\n", (void*)frame);
    sys_panic("Prefetch abort");
}

void arch_data_abort_handler(unsigned int *frame)
{
    unsigned int far;
    asm volatile("mrc p15, 0, %0, c6, c0, 0" : "=r" (far));
    dprintf("EXCEPTION: %s at 0x%08x\n", exc_names[4], far);
    dprintf("Frame at %p\n", (void*)frame);
    sys_panic("Data abort");
}

void arch_unused_handler(unsigned int *frame)
{
    (void)frame;
    dprintf("EXCEPTION: %s\n", exc_names[5]);
}

static volatile unsigned int tick_count = 0;

void arch_irq_handler(unsigned int *frame)
{
    unsigned int id;

    (void)frame;
    id = *(volatile unsigned int *)0x0800100C;

    if (id == 30) {
        tick_count++;
        asm volatile("mcr p15, 0, %0, c14, c2, 1" : : "r" (1));
        asm volatile("mcr p15, 0, %0, c14, c2, 0" : : "r" (0x989680));
        if (tick_count % 10 == 0) {
            static int msg_count = 0;
            dprintf("jcore: tick %d (frame=%p)\n", tick_count++, (void*)frame);
            if (msg_count++ < 3)
                dprintf("jcore: context switch would happen here\n");
        }
    }

    *(volatile unsigned int *)0x08001010 = id;
}

void arch_fiq_handler(unsigned int *frame)
{
    (void)frame;
}
