#include "pinmux.h"

static void set_pin_mode(uint32_t addr, uint32_t mode) {
    volatile uint32_t *reg = (volatile uint32_t *)addr;
    uint32_t val = *reg;
    val &= ~0x0F; /* Clear mode bits */
    val |= mode;
    *reg = val;
}

void pinmux_init_mmc2(void) {
    /* Configure MMC2 pins to Mode 1 (MMC2 function) */
    set_pin_mode(CONF_MMC2_CLK, PINMUX_MODE_MMC2);
    set_pin_mode(CONF_MMC2_CMD, PINMUX_MODE_MMC2);
    set_pin_mode(CONF_MMC2_DAT0, PINMUX_MODE_MMC2);
    set_pin_mode(CONF_MMC2_DAT1, PINMUX_MODE_MMC2);
    set_pin_mode(CONF_MMC2_DAT2, PINMUX_MODE_MMC2);
    set_pin_mode(CONF_MMC2_DAT3, PINMUX_MODE_MMC2);
}
