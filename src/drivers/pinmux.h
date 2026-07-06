#ifndef PINMUX_H
#define PINMUX_H

#include <stdint.h>

/* AM572x Control Module bases
   Pad config registers are at PADCONF_OFFSET within CTRL_MODULE_CORE */
#define CTRL_MODULE_CORE_BASE  0x4A002000
#define PADCONF_OFFSET         0x1400
#define PADCONF_BASE           (CTRL_MODULE_CORE_BASE + PADCONF_OFFSET)

/* MMC2 Pin Offsets (from AM572x TRM padconf map, relative to PADCONF_BASE) */
#define CONF_MMC2_CLK         (PADCONF_BASE + 0x05C0)
#define CONF_MMC2_CMD         (PADCONF_BASE + 0x05C4)
#define CONF_MMC2_DAT0        (PADCONF_BASE + 0x05C8)
#define CONF_MMC2_DAT1        (PADCONF_BASE + 0x05CC)
#define CONF_MMC2_DAT2        (PADCONF_BASE + 0x05D0)
#define CONF_MMC2_DAT3        (PADCONF_BASE + 0x05D4)

/* HDMI Pin Offsets (from AM572x TRM padconf map, relative to PADCONF_BASE) */
#define CONF_HDMI_TX_HPD      (PADCONF_BASE + 0x00D8)
#define CONF_HDMI_TX_CEC      (PADCONF_BASE + 0x00DC)
#define CONF_HDMI_TX_DDC_SCL  (PADCONF_BASE + 0x00E0)
#define CONF_HDMI_TX_DDC_SDA  (PADCONF_BASE + 0x00E4)

/* Pinmux Mode Select (Bits 3:0) */
#define PINMUX_MODE_MMC2      0x01  /* Mode 1 = MMC2 function */
#define PINMUX_MODE_HDMI      0x00  /* Mode 0 = HDMI function */

/* Pad config register bitfields */
#define PINMUX_INPUT_EN       (1U << 8)
#define PINMUX_PULL_EN        (1U << 11)
#define PINMUX_PULL_UP_SEL    (1U << 12)

/* Pull-up/Pull-down / Receiver Enable (Bits 17:16, 5) */
#define PINMUX_PULL_UP        (1U << 17)
#define PINMUX_PULL_DOWN      (0U << 17)
#define PINMUX_RECV_EN        (1U << 5)

void pinmux_init_mmc2(void);
void pinmux_init_hdmi(void);

#endif /* PINMUX_H */