#include "pinmux.h"

static void set_pin_mode(uint32_t addr, uint32_t mode) {
    volatile uint32_t *reg = (volatile uint32_t *)addr;
    uint32_t val = *reg;
    val &= ~0x0F; /* Clear mode bits */
    val |= mode;
    *reg = val;
}

static void set_pad_config(uint32_t addr, uint32_t config) {
    volatile uint32_t *reg = (volatile uint32_t *)addr;
    *reg = config;
}

void pinmux_init_uart3(void) {
    /* Configure UART3 TX/RX pins */
    set_pin_mode(0x44E101A0, 0x1); /* UART3_TX */
    set_pin_mode(0x44E101A4, 0x1); /* UART3_RX */
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

void pinmux_init_hdmi(void) {
    /* HPD: input, pull-down */
    set_pad_config(CONF_HDMI_TX_HPD,
        PINMUX_MODE_HDMI | PINMUX_INPUT_EN | PINMUX_PULL_EN);

    /* CEC: open-drain I/O, pull-up enabled */
    set_pad_config(CONF_HDMI_TX_CEC,
        PINMUX_MODE_HDMI | PINMUX_INPUT_EN | PINMUX_PULL_EN | PINMUX_PULL_UP_SEL);

    /* DDC_SCL: open-drain I/O, pull-up enabled */
    set_pad_config(CONF_HDMI_TX_DDC_SCL,
        PINMUX_MODE_HDMI | PINMUX_INPUT_EN | PINMUX_PULL_EN | PINMUX_PULL_UP_SEL);

    /* DDC_SDA: open-drain I/O, pull-up enabled */
    set_pad_config(CONF_HDMI_TX_DDC_SDA,
        PINMUX_MODE_HDMI | PINMUX_INPUT_EN | PINMUX_PULL_EN | PINMUX_PULL_UP_SEL);
}
