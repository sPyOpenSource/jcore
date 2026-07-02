#include "llama2.h"

#ifdef QEMU
#define UART_BASE          0x09000000

static void uart_putc(char c) {
    volatile unsigned int *fr = (volatile unsigned int *)(UART_BASE + 0x018);
    volatile unsigned char *dr = (volatile unsigned char *)(UART_BASE + 0x000);
    while (*fr & (1 << 5));
    *dr = (unsigned char)c;
}

static void uart_init(void) {
    volatile unsigned int *cr = (volatile unsigned int *)(UART_BASE + 0x030);
    volatile unsigned int *lcr_h = (volatile unsigned int *)(UART_BASE + 0x02C);
    *cr = 0;
    *lcr_h = (3 << 5) | (1 << 4);
    *cr = (1 << 0) | (1 << 8) | (1 << 9);
}
#else
#define UART3_BASE         0x49020000

static void uart_putc(char c) {
    volatile unsigned int *lsr = (volatile unsigned int *)(UART3_BASE + 0x14);
    volatile unsigned char *thr = (volatile unsigned char *)(UART3_BASE + 0x00);
    while (!(*lsr & (1 << 5)));
    *thr = (unsigned char)c;
}

static void uart_init(void) {}
#endif

static void uart_puts(const char *s) {
    while (*s) {
        if (*s == '\n') uart_putc('\r');
        uart_putc(*s++);
    }
}

extern const char _binary_model_bin_start[];

void main(void) {
    uart_init();
    uart_puts("JX boot: OK\n");

    uart_puts("loading model... ");
    Transformer t;
    int rc = build_transformer(&t, _binary_model_bin_start);
    if (rc != 0) {
        uart_puts("FAIL\n");
        return;
    }
    uart_puts("OK\ngenerating...\n");

    generate(&t, 256, uart_putc);

    uart_puts("\ndone\n");

#ifndef QEMU
#define GPIO4_BASE         0x4805D000
#define GPIO_OE            0x134
#define GPIO_SETDATAOUT    0x194
#define GPIO_CLEARDATAOUT  0x190
#define LED_PIN            (1 << 22)

    volatile unsigned int *gpio_oe = (volatile unsigned int *)(GPIO4_BASE + GPIO_OE);
    *gpio_oe &= ~LED_PIN;
    volatile unsigned int *set_reg = (volatile unsigned int *)(GPIO4_BASE + GPIO_SETDATAOUT);
    volatile unsigned int *clear_reg = (volatile unsigned int *)(GPIO4_BASE + GPIO_CLEARDATAOUT);
    while (1) {
        *set_reg = LED_PIN;
        for (volatile unsigned int i = 0; i < 500000; i++);
        *clear_reg = LED_PIN;
        for (volatile unsigned int i = 0; i < 500000; i++);
    }
#else
    while (1);
#endif
}
