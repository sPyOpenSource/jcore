#ifndef PRCM_H
#define PRCM_H

#include <stdint.h>

/* AM572x CM_L4PER base address */
#define CM_L4PER_BASE            0x44E00000
#define CM_L4PER_MMC2_CLKCTRL    (CM_L4PER_BASE + 0x150)

/* CLKCTRL register bits */
#define CLKCTRL_MODULE_EN        (1U << 0)
#define CLKCTRL_IDLEST_MASK      (3U << 30)
#define CLKCTRL_IDLEST_FUNC      (0U << 30)

void prcm_enable_mmc2(void);

#endif /* PRCM_H */
