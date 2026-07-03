#include "types.h"
#include "misc.h"
#include "config.h"

#define TIMER_IRQ 30

#define CNTFRQ  0x00
#define CNTPCT  0x00
#define CNTP_CTL 0x00
#define CNTP_TVAL 0x00

static void timer_reg_write(int reg, unsigned int val)
{
    switch (reg) {
    case 0:
        asm volatile("mcr p15, 0, %0, c14, c0, 0" : : "r" (val));
        break;
    case 1:
        asm volatile("mcr p15, 0, %0, c14, c2, 0" : : "r" (val));
        break;
    case 2:
        asm volatile("mcr p15, 0, %0, c14, c2, 1" : : "r" (val));
        break;
    }
}

static unsigned int timer_reg_read(int reg)
{
    unsigned int val = 0;
    switch (reg) {
    case 0:
        asm volatile("mrc p15, 0, %0, c14, c0, 0" : "=r" (val));
        break;
    case 1:
        asm volatile("mrc p15, 0, %0, c14, c2, 0" : "=r" (val));
        break;
    case 2:
        asm volatile("mrc p15, 0, %0, c14, c2, 1" : "=r" (val));
        break;
    }
    return val;
}

static unsigned int timer_reload_value = 10000000;

void setTimer(void)
{
    unsigned int freq;
    unsigned int ticks;

    freq = timer_reg_read(0);
    if (freq == 0)
        freq = 100000000;

    ticks = freq / TIMER_HZ;
    timer_reload_value = ticks;

    timer_reg_write(1, ticks);
    timer_reg_write(2, 1);
}

extern void gic_enable_irq(int irq);

void arch_timer_init(void)
{
    setTimer();
    gic_enable_irq(TIMER_IRQ);
}

void arch_timer_tick(void)
{
    timer_reg_write(2, 1);
    timer_reg_write(1, timer_reload_value);
}
