#ifndef PINMUX_H
#define PINMUX_H

#include <stdint.h>

/* AM572x Pinmux register base (CTRL_MODULE_CORE) */
#define CTRL_MODULE_CORE_BASE  0x4A002000

/* MMC2 Pin Offsets (from AM572x TRM) - VERIFY THESE AGAINST BBAI SCHEMATIC */
#define CONF_MMC2_CLK         (CTRL_MODULE_CORE_BASE + 0x05C0)
#define CONF_MMC2_CMD         (CTRL_MODULE_CORE_BASE + 0x05C4)
#define CONF_MMC2_DAT0        (CTRL_MODULE_CORE_BASE + 0x05C8)
#define CONF_MMC2_DAT1        (CTRL_MODULE_CORE_BASE + 0x05CC)
#define CONF_MMC2_DAT2        (CTRL_MODULE_CORE_BASE + 0x05D0)
#define CONF_MMC2_DAT3        (CTRL_MODULE_CORE_BASE + 0x05D4)

/* Pinmux Mode Select (Bits 3:0) */
#define PINMUX_MODE_MMC2      0x01  /* Mode 1 = MMC2 function */

/* Pull-up/Pull-down / Receiver Enable (Bits 17:16, 5) */
#define PINMUX_PULL_UP        (1U << 17)
#define PINMUX_PULL_DOWN      (0U << 17)
#define PINMUX_RECV_EN        (1U << 5)

void pinmux_init_mmc2(void);

#endif /* PINMUX_H */