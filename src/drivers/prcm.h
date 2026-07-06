#ifndef PRCM_H
#define PRCM_H

#include <stdint.h>

/* AM572x CM_CORE_AON (DSS clock) */
#define CM_CORE_AON_BASE         0x4A005200
#define CM_DSS_DSS_CLKCTRL       (CM_CORE_AON_BASE + 0x00)

/* AM572x CM_L4PER base address */
#define CM_L4PER_BASE            0x44E00000
#define CM_L4PER_MMC2_CLKCTRL    (CM_L4PER_BASE + 0x150)

/* CLKCTRL register bits (common format for AM5728) */
#define CLKCTRL_MODULE_EN        (1U << 0)
#define CLKCTRL_IDLEST_MASK      (3U << 30)
#define CLKCTRL_IDLEST_FUNC      (0U << 30)

/* DSS CLKCTRL: MODULEMODE at [1:0], IDLEST at [17:16] */
#define DSS_CLKCTRL_ENABLE       (2U << 0)
#define DSS_CLKCTRL_IDLEST_MASK  (3U << 16)
#define DSS_CLKCTRL_IDLEST_FUNC  (0U << 16)

void prcm_enable_mmc2(void);
void prcm_enable_dss(void);

#endif /* PRCM_H */
