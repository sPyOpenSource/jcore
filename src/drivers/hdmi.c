#include "hdmi.h"
#include "dss.h"
#include "misc.h"
#include <stdint.h>

/* Register access helpers */
static volatile uint32_t* wp_reg(int offset) {
    return (volatile uint32_t*)(HDMI_WP_BASE + offset);
}

static volatile uint32_t* pll_reg(int offset) {
    return (volatile uint32_t*)(HDMI_PLL_BASE + offset);
}

static volatile uint32_t* phy_reg(int offset) {
    return (volatile uint32_t*)(HDMI_PHY_BASE + offset);
}

static volatile uint32_t* core_reg(int offset) {
    return (volatile uint32_t*)(HDMI_CORE_BASE + offset);
}

static volatile uint8_t* core_reg8(int offset) {
    return (volatile uint8_t*)(HDMI_CORE_BASE + offset);
}

static void hdmi_wp_set_phy_pwr(int cmd) {
    uint32_t v = *wp_reg(HDMI_WP_PWR_CTRL);
    v &= ~(3U << 6);
    v |= cmd;
    *wp_reg(HDMI_WP_PWR_CTRL) = v;
}

static int hdmi_wp_get_phy_status(void) {
    return (*wp_reg(HDMI_WP_PWR_CTRL) >> 4) & 3;
}

static void hdmi_pll_configure(void) {
    /* Configure HDMI PLL for ~25 MHz TMDS clock (640x480 @60Hz)
       Reference clock assumed = 24 MHz (typical onboard XTAL)
       
       Formula: TMDS = (REFCLK * REGM) / (REGN * HSDIV * OUTDIV)
       VCO = REFCLK * REGM / REGN  (must be in PLL's valid range ~1-4 GHz)
       
       Using: REGN=1, REGM=105, HSDIV=20, OUTDIV=5
       VCO   = 24 * 105 / 1 = 2520 MHz  (in range)
       TMDS  = 2520 / (20 * 5) = 25.2 MHz  (close to 25.175 MHz target)
       
       These values WILL need tuning on real hardware.
    */

    /* CFG1: REGN[4:0]=1, REGM[23:8]=105 */
    *pll_reg(PLLCTRL_CFG1) = (1 << 0) | (105 << 8);

    /* CFG2: HSDIV[5:0]=20 (post-divider) */
    *pll_reg(PLLCTRL_CFG2) = 20;

    /* CFG3: additional divider config (default) */
    *pll_reg(PLLCTRL_CFG3) = 0;

    /* CFG4: OUTDIV[10:8]=5, enable output clocks */
    *pll_reg(PLLCTRL_CFG4) = (5 << 8) | PLL_CFG4_PLLCLKOUTEN | PLL_CFG4_PLLDCOCLKOUTEN;

    /* Take PLL out of bypass */
    *pll_reg(PLLCTRL_PLL_CONTROL) = 0;

    /* Trigger GO bit to start locking */
    *pll_reg(PLLCTRL_PLL_GO) = 1;
}

static int hdmi_pll_wait_lock(void) {
    int timeout = 10000;
    while (timeout--) {
        uint32_t status = *pll_reg(PLLCTRL_PLL_STATUS);
        if (status & PLL_LOCK)
            return 0;
    }
    return -1;
}

static void hdmi_phy_init(void) {
    /* Configure PHY digital control */
    *phy_reg(HDMI_TXPHY_DIGITAL_CTRL) = 0;

    /* Enable PHY LDO */
    *phy_reg(HDMI_TXPHY_POWER_CTRL) |= PHY_POWER_LDO_EN;
}

static void hdmi_core_set_timings(void) {
    uint16_t hactive = g_display.width;
    uint16_t hblank = g_display.hfp + g_display.hsw + g_display.hbp;
    uint16_t vactive = g_display.height;
    uint16_t vblank = g_display.vfp + g_display.vsw + g_display.vbp;

    /* Set video input config: HDMI mode, positive sync polarity */
    *core_reg(HDMI_CORE_FC_INVIDCONF) =
        FC_INVIDCONF_HDMI_MODE |
        FC_INVIDCONF_VBLANK_OSC |
        FC_INVIDCONF_HSYNC_POL_HIGH |
        FC_INVIDCONF_VSYNC_POL_HIGH;

    /* Horizontal active width */
    *core_reg8(HDMI_CORE_FC_INHACTIV0) = hactive & 0xFF;
    *core_reg8(HDMI_CORE_FC_INHACTIV1) = (hactive >> 8) & 0x0F;

    /* Horizontal blank width */
    *core_reg8(HDMI_CORE_FC_INHBLANK0) = hblank & 0xFF;
    *core_reg8(HDMI_CORE_FC_INHBLANK1) = (hblank >> 8) & 0x0F;

    /* Horizontal sync delay (HSYNC front porch = hfp) */
    *core_reg8(HDMI_CORE_FC_HSYNCINDELAY0) = g_display.hfp & 0xFF;
    *core_reg8(HDMI_CORE_FC_HSYNCINDELAY1) = (g_display.hfp >> 8) & 0x0F;

    /* Horizontal sync width */
    *core_reg8(HDMI_CORE_FC_HSYNCINWIDTH0) = g_display.hsw & 0xFF;
    *core_reg8(HDMI_CORE_FC_HSYNCINWIDTH1) = (g_display.hsw >> 8) & 0x0F;

    /* Vertical active height */
    *core_reg8(HDMI_CORE_FC_INVACTIV0) = vactive & 0xFF;
    *core_reg8(HDMI_CORE_FC_INVACTIV1) = (vactive >> 8) & 0x0F;

    /* Vertical blank height */
    *core_reg8(HDMI_CORE_FC_INVBLANK0) = vblank & 0xFF;
    *core_reg8(HDMI_CORE_FC_INVBLANK1) = (vblank >> 8) & 0x0F;

    /* Vertical sync delay (VSYNC front porch = vfp) */
    *core_reg8(HDMI_CORE_FC_VSYNCINDELAY0) = g_display.vfp & 0xFF;
    *core_reg8(HDMI_CORE_FC_VSYNCINDELAY1) = (g_display.vfp >> 8) & 0x0F;

    /* Vertical sync width */
    *core_reg8(HDMI_CORE_FC_VSYNCINWIDTH0) = g_display.vsw & 0xFF;
    *core_reg8(HDMI_CORE_FC_VSYNCINWIDTH1) = (g_display.vsw >> 8) & 0x0F;

    dprintf("HDMI: timings set %dx%d (Hblank=%d, Vblank=%d)\n",
            hactive, vactive, hblank, vblank);
}

static void hdmi_wp_configure(void) {
    /* Set video source from DISPC GFX pipeline */
    *wp_reg(HDMI_WP_VIDEO_CFG) = WP_VIDEO_SRC_VID1 | WP_VIDEO_BLANK_INSERT;

    /* Set video size */
    *wp_reg(HDMI_WP_VIDEO_SIZE) = ((uint32_t)g_display.height << 16) | g_display.width;

    /* Set timing registers (HBP, HFP, HSW packed) */
    uint32_t h_timing = (g_display.hbp << 22) |
                        (g_display.hfp << 12) |
                        (g_display.hsw << 0);
    *wp_reg(HDMI_WP_VIDEO_TIMING_H) = h_timing;

    uint32_t v_timing = (g_display.vbp << 22) |
                        (g_display.vfp << 12) |
                        (g_display.vsw << 0);
    *wp_reg(HDMI_WP_VIDEO_TIMING_V) = v_timing;

    /* Clock divider: bypass divide by 1 */
    *wp_reg(HDMI_WP_CLK) = 0;

    dprintf("HDMI: WP configured\n");
}

void hdmi_init(void) {
    dprintf("HDMI: init start\n");

    /* Reset WP */
    *wp_reg(HDMI_WP_SYSCONFIG) = 2;
    /* Note: HDMI WP has no SYSSTATUS bit, just proceed after delay */

    /* === Phase 1: PHY power up (LDO first) === */
    dprintf("HDMI: powering PHY LDO...\n");
    hdmi_wp_set_phy_pwr(WP_PWR_CMD_LDO_ON);
    volatile int delay;
    for (delay = 0; delay < 100000; delay++);
    dprintf("HDMI: PHY status = %d\n", hdmi_wp_get_phy_status());

    /* === Phase 2: PLL configuration === */
    dprintf("HDMI: configuring PLL...\n");
    hdmi_pll_configure();
    if (hdmi_pll_wait_lock() == 0) {
        dprintf("HDMI: PLL locked\n");
    } else {
        dprintf("HDMI: PLL lock timeout (using bypass)\n");
    }

    /* === Phase 3: PHY TX power up === */
    dprintf("HDMI: powering PHY TX...\n");
    hdmi_wp_set_phy_pwr(WP_PWR_CMD_TX_ON);
    for (delay = 0; delay < 100000; delay++);
    dprintf("HDMI: PHY status = %d\n", hdmi_wp_get_phy_status());

    /* === Phase 3b: PHY internal init === */
    hdmi_phy_init();

    /* === Phase 4: Configure HDMI core frame composer === */
    hdmi_core_set_timings();

    /* === Phase 5: Configure WP === */
    hdmi_wp_configure();

    dprintf("HDMI: init done\n");
}

int hdmi_read_edid(uint8_t *buf, int len) {
    int i;

    /* Configure I2CM divider based on pixel clock */
    *core_reg(HDMI_CORE_I2CM_DIV) = 0;

    /* Set EDID address (0x50) */
    *core_reg(HDMI_CORE_I2CM_ADDRESS) = EDID_I2C_ADDR;

    /* Read 128-byte EDID block (multiple I2C transactions) */
    for (i = 0; i < len && i < 128; i++) {
        /* Start read transaction */
        *core_reg(HDMI_CORE_I2CM_OPERATION) = I2CM_READ;

        volatile int timeout = 1000;
        while (timeout--);

        /* Read data byte */
        buf[i] = (uint8_t)*core_reg(HDMI_CORE_I2CM_DATAI);
    }

    dprintf("HDMI: read %d EDID bytes\n", i);
    return i;
}

void hdmi_wait_vsync(void) {
    /* Poll for VSYNC interrupt status on DISPC */
    volatile uint32_t *irqstatus = (volatile uint32_t*)(DISPC_BASE + DISPC_IRQSTATUS);
    while (!(*irqstatus & 1));
    *irqstatus = 1;
}
