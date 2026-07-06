#ifndef DSS_H
#define DSS_H

#include <stdint.h>

/* AM5728 DSS base addresses */
#define DSS_BASE                0x58000000
#define DISPC_BASE              0x58001000
#define HDMI_WP_BASE            0x58060000
#define HDMI_PLL_BASE           0x58062000
#define HDMI_PHY_BASE           0x58064000
#define HDMI_CORE_BASE          0x58080000

/* DSS wrapper registers (offset from DSS_BASE) */
#define DSS_REVISION            0x0000
#define DSS_SYSCONFIG           0x0010
#define DSS_SYSSTATUS           0x0014
#define DSS_CONTROL             0x0040
#define DSS_STATUS              0x0044

/* DSS_CONTROL bits */
#define DSS_CTRL_CLK_CONTROL    (1U << 0)
#define DSS_CTRL_DPI_SELECT_DPI0 (0U << 2)
#define DSS_CTRL_DPI_SELECT_DPI1 (1U << 2)
#define DSS_CTRL_DPI_SELECT_DPI2 (2U << 2)

/* DISPC common registers (offset from DISPC_BASE) */
#define DISPC_REVISION          0x0000
#define DISPC_SYSCONFIG         0x0010
#define DISPC_SYSSTATUS         0x0014
#define DISPC_IRQSTATUS         0x0018
#define DISPC_IRQENABLE         0x001C
#define DISPC_CONTROL           0x0040
#define DISPC_CONFIG            0x0044
#define DISPC_CAPABLE           0x0048
#define DISPC_CONTROL2          0x0238
#define DISPC_DIVISOR           0x0804

/* DISPC_CONTROL bits */
#define DISPC_CTRL_LCDENABLE    (1U << 0)
#define DISPC_CTRL_GOLCD        (1U << 1)

/* DISPC GFX pipeline registers (offset from DISPC_BASE) */
#define DISPC_GFX_BA0           0x0080
#define DISPC_GFX_BA1           0x0084
#define DISPC_GFX_POSITION      0x0088
#define DISPC_GFX_SIZE          0x008C
#define DISPC_GFX_ATTRIBUTES    0x00A0
#define DISPC_GFX_FIFO_THRESHOLD 0x00A4
#define DISPC_GFX_ROW_INC       0x00A8
#define DISPC_GFX_PIXEL_INC     0x00AC
#define DISPC_GFX_WINDOW_SKIP   0x00B0
#define DISPC_GFX_TABLE_BA      0x00B4
#define DISPC_GFX_ATTRIBUTES2   0x00B8

/* DISPC_GFX_ATTRIBUTES bitfields */
#define GFX_FORMAT_RGB16        (1U << 2)
#define GFX_FORMAT_RGB24P       (2U << 2)
#define GFX_FORMAT_RGB24_32     (3U << 2)
#define GFX_FORMAT_ARGB32       (6U << 2)
#define GFX_BURST               (1U << 7)
#define GFX_CHANNELOUT_GFX      (0U << 0)
#define GFX_CHANNELOUT_VID1     (1U << 0)
#define GFX_ZORDER_0            (0U << 12)
#define GFX_ZORDER_1            (1U << 12)
#define GFX_ZORDER_2            (2U << 12)
#define GFX_ZORDER_3            (3U << 12)

/* DISPC_GFX_ATTRIBUTES2 */
#define GFX_ENABLE              (1U << 0)

/* Framebuffer config */
#define FB_WIDTH                640
#define FB_HEIGHT               480
#define FB_BPP                  4
#define FB_STRIDE               (FB_WIDTH * FB_BPP)
#define FB_SIZE                 (FB_HEIGHT * FB_STRIDE)

/* Framebuffer address (DDR, after kernel) */
#define FB_ADDR                 0x81000000

/* Display timings for 640x480 @60Hz */
#define DISPLAY_HACTIVE         640
#define DISPLAY_HFP             16
#define DISPLAY_HSW             96
#define DISPLAY_HBP             48
#define DISPLAY_HTOTAL          800
#define DISPLAY_VACTIVE         480
#define DISPLAY_VFP             10
#define DISPLAY_VSW             2
#define DISPLAY_VBP             33
#define DISPLAY_VTOTAL          525

typedef struct display_info {
    uint16_t width;
    uint16_t height;
    uint16_t hfp;
    uint16_t hsw;
    uint16_t hbp;
    uint16_t vfp;
    uint16_t vsw;
    uint16_t vbp;
    uint32_t pixel_clock_hz;
    uint32_t framebuffer_addr;
} display_info_t;

extern display_info_t g_display;

void dss_init(void);
void dss_set_framebuffer(uint32_t addr);
void dss_update(void);
void dss_fill_colorbar(void);

#endif
