#ifndef HDMI_H
#define HDMI_H

#include <stdint.h>

/* === HDMI Wrapper (WP) registers (offset from HDMI_WP_BASE) === */
#define HDMI_WP_REVISION            0x0000
#define HDMI_WP_SYSCONFIG           0x0010
#define HDMI_WP_IRQSTATUS_RAW       0x0024
#define HDMI_WP_IRQSTATUS           0x0028
#define HDMI_WP_IRQENABLE_SET       0x002C
#define HDMI_WP_IRQENABLE_CLR       0x0030
#define HDMI_WP_PWR_CTRL            0x0040
#define HDMI_WP_DEBOUNCE            0x0044
#define HDMI_WP_VIDEO_CFG           0x0050
#define HDMI_WP_VIDEO_SIZE          0x0060
#define HDMI_WP_VIDEO_TIMING_H      0x0068
#define HDMI_WP_VIDEO_TIMING_V      0x006C
#define HDMI_WP_CLK                 0x0070

/* WP_PWR_CTRL fields */
#define WP_PWR_CMD_OFF              (0U << 6)
#define WP_PWR_CMD_LDO_ON           (1U << 6)
#define WP_PWR_CMD_TX_ON            (2U << 6)
#define WP_PWR_STATUS_OFF           (0U << 4)
#define WP_PWR_STATUS_LDO_ON        (1U << 4)
#define WP_PWR_STATUS_TX_ON         (2U << 4)

/* WP_VIDEO_CFG fields */
#define WP_VIDEO_SRC_SA             (0U << 2)
#define WP_VIDEO_SRC_VID1           (1U << 2)
#define WP_VIDEO_SRC_VID2           (2U << 2)
#define WP_VIDEO_SRC_VID3           (3U << 2)
#define WP_VIDEO_BLANK_INSERT       (1U << 0)
#define WP_VIDEO_PACKET_MODE        (1U << 1)
#define WP_VIDEO_BLANK_END          (1U << 4)

/* WP_CLK fields */
#define WP_CLK_CLK_DIV_MASK         0x0F

/* === HDMI PLL registers (offset from HDMI_PLL_BASE) === */
#define PLLCTRL_PLL_CONTROL         0x0000
#define PLLCTRL_PLL_STATUS          0x0004
#define PLLCTRL_PLL_GO              0x0008
#define PLLCTRL_CFG1                0x000C
#define PLLCTRL_CFG2                0x0010
#define PLLCTRL_CFG3                0x0014
#define PLLCTRL_CFG4                0x0020

/* PLL_STATUS bits */
#define PLL_LOCK                    (1U << 0)
#define PLL_BYPASS                  (1U << 1)

/* PLL_CONTROL bits */
#define PLL_CTRL_BYPASS             (1U << 0)

/* PLL_CFG1 fields */
#define PLL_CFG1_REF_DIV_MASK       0x1F
#define PLL_CFG1_REF_DIV_SHIFT      0
#define PLL_CFG1_MULT_MASK          0xFFFF
#define PLL_CFG1_MULT_SHIFT         8

/* PLL_CFG2 fields */
#define PLL_CFG2_HSDIV_MASK         0x3F
#define PLL_CFG2_HSDIV_SHIFT        0

/* PLL_CFG4 fields */
#define PLL_CFG4_CLKDCOLDOEN        0
#define PLL_CFG4_PLLDCOCLKOUTEN     (1U << 2)
#define PLL_CFG4_PLLCLKOUTEN        (1U << 3)
#define PLL_CFG4_OUTPUTDIV_MASK     0x7
#define PLL_CFG4_OUTPUTDIV_SHIFT    8

/* === HDMI PHY registers (offset from HDMI_PHY_BASE) === */
#define HDMI_TXPHY_TX_CTRL          0x0000
#define HDMI_TXPHY_DIGITAL_CTRL     0x0004
#define HDMI_TXPHY_POWER_CTRL       0x0008
#define HDMI_TXPHY_CLK              0x000C

/* PHY_POWER_CTRL */
#define PHY_POWER_LDO_EN            (1U << 0)
#define PHY_POWER_TX_EN             (1U << 1)
#define PHY_POWER_PLL_EN            (1U << 2)

/* === HDMI Core (IP) registers (offset from HDMI_CORE_BASE) === */
/* Frame composer - video input */
#define HDMI_CORE_FC_INVIDCONF      0x4000
#define HDMI_CORE_FC_INHACTIV0      0x4004
#define HDMI_CORE_FC_INHACTIV1      0x4008
#define HDMI_CORE_FC_INHBLANK0      0x400C
#define HDMI_CORE_FC_INHBLANK1      0x4010
#define HDMI_CORE_FC_INVACTIV0      0x4058
#define HDMI_CORE_FC_INVACTIV1      0x405C
#define HDMI_CORE_FC_INVBLANK0      0x4060
#define HDMI_CORE_FC_INVBLANK1      0x4064
#define HDMI_CORE_FC_VSYNCINDELAY0  0x4068
#define HDMI_CORE_FC_VSYNCINDELAY1  0x406C
#define HDMI_CORE_FC_VSYNCINWIDTH0  0x4070
#define HDMI_CORE_FC_VSYNCINWIDTH1  0x4074
#define HDMI_CORE_FC_HSYNCINDELAY0  0x40A0
#define HDMI_CORE_FC_HSYNCINDELAY1  0x40A4
#define HDMI_CORE_FC_HSYNCINWIDTH0  0x40AC
#define HDMI_CORE_FC_HSYNCINWIDTH1  0x40B0

/* FC_INVIDCONF fields */
#define FC_INVIDCONF_HDMI_MODE      (1U << 3)
#define FC_INVIDCONF_DVI_MODE       (0U << 3)
#define FC_INVIDCONF_VBLANK_OSC     (1U << 2)
#define FC_INVIDCONF_HSYNC_POL_HIGH (1U << 4)
#define FC_INVIDCONF_VSYNC_POL_HIGH (1U << 5)

/* AVI InfoFrame */
#define HDMI_CORE_AVI_DB0           0x4400
#define HDMI_CORE_AVI_DB1           0x4401
#define HDMI_CORE_AVI_DB2           0x4402
#define HDMI_CORE_AVI_DB3           0x4403
#define HDMI_CORE_AVI_DB4           0x4404
#define HDMI_CORE_AVI_DB5           0x4405

/* I2C master for DDC (EDID) */
#define HDMI_CORE_I2CM_DATAI        0x4920
#define HDMI_CORE_I2CM_ADDRESS      0x4924
#define HDMI_CORE_I2CM_DIV          0x4940
#define HDMI_CORE_I2CM_OPERATION    0x4980

/* I2CM_OPERATION */
#define I2CM_READ                   (1U << 0)
#define I2CM_STOP                   (1U << 1)

/* EDID address */
#define EDID_I2C_ADDR               0x50

/* API */
void hdmi_init(void);
int hdmi_read_edid(uint8_t *buf, int len);
void hdmi_set_timings(uint16_t hactive, uint16_t hfp, uint16_t hsw, uint16_t hbp,
                      uint16_t vactive, uint16_t vfp, uint16_t vsw, uint16_t vbp);
void hdmi_wait_vsync(void);

#endif /* HDMI_H */
