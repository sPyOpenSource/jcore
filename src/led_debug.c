#include "includes/led_debug.h"

#ifndef QEMU
#define GPIO4_BASE         0x4805D000
#define GPIO_OE            0x134
#define GPIO_SETDATAOUT    0x194
#define GPIO_CLEARDATAOUT  0x190
#define LED_PIN            (1 << 22)

static void led_init(void) {
    volatile unsigned int *gpio_oe = (volatile unsigned int *)(GPIO4_BASE + GPIO_OE);
    *gpio_oe &= ~LED_PIN;
}

void led_on(void) {
    volatile unsigned int *set_reg = (volatile unsigned int *)(GPIO4_BASE + GPIO_SETDATAOUT);
    *set_reg = LED_PIN;
}

void led_off(void) {
    volatile unsigned int *clear_reg = (volatile unsigned int *)(GPIO4_BASE + GPIO_CLEARDATAOUT);
    *clear_reg = LED_PIN;
}

void led_signal(int count) {
    led_init();
    if (count <= 0) {
        led_off();
        return;
    }
    for (int i = 0; i < count; i++) {
        led_on();
        for (volatile unsigned int j = 0; j < 500000; j++);
        led_off();
        for (volatile unsigned int j = 0; j < 500000; j++);
    }
}
#endif
