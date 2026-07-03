#include "types.h"
#include "misc.h"

#define GIC_BASE       0x08000000
#define GICD_BASE      (GIC_BASE + 0x0000)
#define GICC_BASE      (GIC_BASE + 0x10000)

#define GICD_CTLR      (GICD_BASE + 0x000)
#define GICD_TYPER     (GICD_BASE + 0x004)
#define GICD_ISENABLER (GICD_BASE + 0x100)
#define GICD_ICENABLER (GICD_BASE + 0x180)
#define GICD_IPRIORITYR (GICD_BASE + 0x400)
#define GICD_ITARGETSR (GICD_BASE + 0x800)
#define GICD_ICFGR     (GICD_BASE + 0xC00)

#define GICC_CTLR      (GICC_BASE + 0x000)
#define GICC_PMR       (GICC_BASE + 0x004)
#define GICC_IAR       (GICC_BASE + 0x00C)
#define GICC_EOIR      (GICC_BASE + 0x010)

#define GICD_CTLR_ENABLE 1
#define GICC_CTLR_ENABLE 1
#define GICC_PMR_PRIORITY 0xF0

void gic_init(void)
{
    volatile unsigned int *d;
    volatile unsigned int *c;
    int i;

    d = (volatile unsigned int *)GICD_CTLR;
    d[0] = 0;
    d = (volatile unsigned int *)GICC_CTLR;
    d[0] = 0;

    for (i = 0; i < 32; i++) {
        volatile unsigned int *pri = (volatile unsigned int *)(GICD_IPRIORITYR + i * 4);
        *pri = 0x80808080;
    }

    for (i = 0; i < 32; i++) {
        volatile unsigned int *target = (volatile unsigned int *)(GICD_ITARGETSR + i * 4);
        *target = 0x01010101;
    }

    d = (volatile unsigned int *)GICD_ICFGR;
    d[0] = 0xAAAAAAAA;

    d = (volatile unsigned int *)GICD_ICENABLER;
    d[0] = 0xFFFFFFFF;

    d = (volatile unsigned int *)GICC_PMR;
    d[0] = GICC_PMR_PRIORITY;

    c = (volatile unsigned int *)GICC_CTLR;
    c[0] = GICC_CTLR_ENABLE;

    d = (volatile unsigned int *)GICD_CTLR;
    d[0] = GICD_CTLR_ENABLE;
}

void gic_enable_irq(int irq)
{
    volatile unsigned int *en = (volatile unsigned int *)(GICD_ISENABLER + (irq / 32) * 4);
    *en = (1 << (irq % 32));
}

void gic_disable_irq(int irq)
{
    volatile unsigned int *dis = (volatile unsigned int *)(GICD_ICENABLER + (irq / 32) * 4);
    *dis = (1 << (irq % 32));
}

unsigned int gic_acknowledge(void)
{
    volatile unsigned int *iar = (volatile unsigned int *)GICC_IAR;
    return *iar;
}

void gic_end_interrupt(unsigned int id)
{
    volatile unsigned int *eoir = (volatile unsigned int *)GICC_EOIR;
    *eoir = id;
}
