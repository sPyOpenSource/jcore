#include "flint_native_rpi.h"
#include "flint_java_string.h"
#include <stdint.h>

/*
 * Raspberry Pi 4 BCM2711 Peripheral Base Addresses
 * For RPi 3, these would be 0x3F000000.
 * For RPi 4, these are 0xFE000000.
 */
#define RPI_PERIPHERAL_BASE 0xFE000000
#define RPI_UART0_BASE      (RPI_PERIPHERAL_BASE + 0x201000)
#define RPI_GPIO_BASE       (RPI_PERIPHERAL_BASE + 0x200000)
#define RPI_SYSTEM_TIMER    0x40000000

/* UART Register Offsets */
#define UART_DR   0x00
#define UART_FR   0x18

/* GPIO Register Offsets */
#define GPIO_SET0 0x1C
#define GPIO_CLR0 0x28

static void rpi_uart_putc(char c) {
    volatile uint32_t *uart_fr = (volatile uint32_t*)(RPI_UART0_BASE + UART_FR);
    volatile uint32_t *uart_dr = (volatile uint32_t*)(RPI_UART0_BASE + UART_DR);
    while ((*uart_fr & 0x20) != 0);
    *uart_dr = (uint32_t)c;
}

static void rpi_gpio_set(uint32_t pin, bool value) {
    volatile uint32_t *gpio_set = (volatile uint32_t*)(RPI_GPIO_BASE + GPIO_SET0);
    volatile uint32_t *gpio_clr = (volatile uint32_t*)(RPI_GPIO_BASE + GPIO_CLR0);
    if (value) {
        *gpio_set = (1 << pin);
    } else {
        *gpio_clr = (1 << pin);
    }
}

jvoid NativeRPi_Print(FNIEnv *env, jstring msg) {
    if (msg == NULL) return;
    const char *str = msg->getAscii();
    if (str) {
        while (*str) {
            rpi_uart_putc(*str++);
        }
    }
}

jlong NativeRPi_CurrentTimeMillis(FNIEnv *env) {
    (void)env;
    uint64_t ticks = *(volatile uint64_t*)RPI_SYSTEM_TIMER;
    return (jlong)(ticks / 54000);
}

jvoid NativeRPi_SetPin(FNIEnv *env, jint pin, jint value) {
    (void)env;
    rpi_gpio_set((uint32_t)pin, value != 0);
}

jint NativeRPi_Random(FNIEnv *env) {
    (void)env;
    static uint32_t seed = 0x12345678;
    seed = seed * 1103515245 + 12345;
    return (jint)(seed & 0x7FFFFFFF);
}
