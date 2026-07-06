#include "dss.h"
#include "misc.h"
#include <stdint.h>

display_info_t g_display = {
    .width = FB_WIDTH,
    .height = FB_HEIGHT,
    .hfp = DISPLAY_HFP,
    .hsw = DISPLAY_HSW,
    .hbp = DISPLAY_HBP,
    .vfp = DISPLAY_VFP,
    .vsw = DISPLAY_VSW,
    .vbp = DISPLAY_VBP,
    .pixel_clock_hz = 25175000,
    .framebuffer_addr = FB_ADDR,
};

static volatile uint32_t* dss_reg(int offset) {
    return (volatile uint32_t*)(DSS_BASE + offset);
}

static volatile uint32_t* dispc_reg(int offset) {
    return (volatile uint32_t*)(DISPC_BASE + offset);
}

void dss_init(void) {
    dprintf("DSS: init start\n");

    /* Reset DSS wrapper */
    *dss_reg(DSS_SYSCONFIG) = 2;
    while (!(*dss_reg(DSS_SYSSTATUS) & 1));

    /* Configure DSS_CONTROL: DPI_OUT0 selected, clock passthrough */
    *dss_reg(DSS_CONTROL) = DSS_CTRL_CLK_CONTROL | DSS_CTRL_DPI_SELECT_DPI0;

    /* Reset DISPC */
    *dispc_reg(DISPC_SYSCONFIG) = 2;
    while (!(*dispc_reg(DISPC_SYSSTATUS) & 1));

    /* DISPC SYSCONFIG: smart-idle, auto-clock gating */
    *dispc_reg(DISPC_SYSCONFIG) = 0;

    /* Mask all interrupts */
    *dispc_reg(DISPC_IRQENABLE) = 0;
    *dispc_reg(DISPC_IRQSTATUS) = 0xFFFFFFFF;

    /* Configure pixel clock divisor:
       pixel_clock = DSS_FCK / PCD
       Assuming DSS_FCK ~200MHz, PCD=8 gives 25MHz (close to 25.175 for 640x480) */
    *dispc_reg(DISPC_DIVISOR) = (8 << 16) | 1;

    dprintf("DSS: init done\n");
}

void dss_set_framebuffer(uint32_t addr) {
    g_display.framebuffer_addr = addr;
    *dispc_reg(DISPC_GFX_BA0) = addr;
    *dispc_reg(DISPC_GFX_BA1) = addr;
}

void dss_update(void) {
    /* Set GFX size and position */
    *dispc_reg(DISPC_GFX_SIZE) = ((uint32_t)g_display.height << 16) | g_display.width;
    *dispc_reg(DISPC_GFX_POSITION) = 0;

    /* Row increment: bytes per scanline */
    *dispc_reg(DISPC_GFX_ROW_INC) = g_display.width * FB_BPP;

    /* Pixel increment: source and destination = bytes per pixel */
    uint32_t pix_inc = (FB_BPP << 16) | FB_BPP;
    *dispc_reg(DISPC_GFX_PIXEL_INC) = pix_inc;

    /* FIFO thresholds: low=0x100, high=0x400 */
    *dispc_reg(DISPC_GFX_FIFO_THRESHOLD) = (0x100 << 16) | 0x400;

    /* Attributes: RGB24_32 format, channel=GFX, burst enabled */
    *dispc_reg(DISPC_GFX_ATTRIBUTES) = GFX_FORMAT_RGB24_32 | GFX_CHANNELOUT_GFX | GFX_BURST | GFX_ZORDER_0;

    /* Enable GFX pipeline */
    *dispc_reg(DISPC_GFX_ATTRIBUTES2) = GFX_ENABLE;

    /* Enable LCD output */
    *dispc_reg(DISPC_CONTROL) |= DISPC_CTRL_LCDENABLE;

    /* Trigger GO bit to apply shadow registers */
    *dispc_reg(DISPC_CONTROL) |= DISPC_CTRL_GOLCD;

    dprintf("DSS: display enabled\n");
}

void dss_fill_colorbar(void) {
    volatile uint32_t *fb = (volatile uint32_t *)g_display.framebuffer_addr;
    int x, y;

    /* 8 vertical color bars */
    static const uint32_t colors[] = {
        0x00FF0000, /* Red */
        0x00FFA500, /* Orange */
        0x00FFFF00, /* Yellow */
        0x0000FF00, /* Green */
        0x000000FF, /* Blue */
        0x004B0082, /* Indigo */
        0x00800080, /* Violet */
        0x00FFFFFF, /* White */
    };

    int bar_w = g_display.width / 8;

    for (y = 0; y < g_display.height; y++) {
        for (x = 0; x < g_display.width; x++) {
            int bar = x / bar_w;
            if (bar > 7) bar = 7;
            fb[y * g_display.width + x] = colors[bar];
        }
    }

    dprintf("DSS: colorbar drawn at 0x%08x (%dx%d)\n",
            g_display.framebuffer_addr, g_display.width, g_display.height);
}

void dss_draw_pixel(int x, int y, uint32_t color) {
    volatile uint32_t *fb = (volatile uint32_t *)g_display.framebuffer_addr;
    if (x >= 0 && x < g_display.width && y >= 0 && y < g_display.height)
        fb[y * g_display.width + x] = color;
}
