#include "prcm.h"

void prcm_enable_mmc2(void) {
    volatile uint32_t *clkctrl = (volatile uint32_t *)CM_L4PER_MMC2_CLKCTRL;

    /* Enable MMC2 module clock */
    *clkctrl |= CLKCTRL_MODULE_EN;

    /* Poll until the module is in FUNC state */
    while ((*clkctrl & CLKCTRL_IDLEST_MASK) != CLKCTRL_IDLEST_FUNC) {
        /* Wait for clock to stabilize */
    }
}

void prcm_enable_dss(void) {
    volatile uint32_t *clkctrl = (volatile uint32_t *)CM_DSS_DSS_CLKCTRL;

    /* Enable DSS module explicitly (MODULEMODE = ENABLED) */
    *clkctrl = (*clkctrl & ~3U) | DSS_CLKCTRL_ENABLE;

    /* Poll IDLEST at [17:16] until functional */
    while ((*clkctrl & DSS_CLKCTRL_IDLEST_MASK) != DSS_CLKCTRL_IDLEST_FUNC) {
        /* Wait for clock to stabilize */
    }
}
