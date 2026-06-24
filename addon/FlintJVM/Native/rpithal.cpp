#include <string.h>
#include <sys/errno.h>
#include <sys/time.h>
#include <stdint.h>

#include "flint.h"
#include "flint_native_interface.h"

/*
 * RPi Native Interface Implementation
 *
 * This file implements the Native Interface (JNI-like abstraction layer)
 * for the Raspberry Pi platform, connecting FlintJVM to the hardware.
 *
 * Functions to implement:
 * - native_uart_putc -> RPi UART driver
 * - native_pin_set -> RPi GPIO driver
 * - native_system_get_time -> get current time
 */

// RPi peripheral base addresses (from BCM2837 datasheet, RPi 3/4)
#define GPIO_BASE      0xFE200000
#define UART0_BASE     0xFE201000
#define MAILBOX_BASE   0xFE00B880  // ARM Local Memory

/*
 * GPIO Peripheral
 */
typedef struct {
    volatile uint32_t GPFSEL[6];   // 0x00 - 0x50
    volatile uint32_t GPSET[2];     // 0x1E - 0x20
    volatile uint32_t GPCLR[2];     // 0x28 - 0x2A
    volatile uint32_t GPLEV[2];     // 0x34 - 0x36
    volatile uint32_t GPEDS;        // 0x3C
    volatile uint32_t GPREN[2];     // 0x64 - 0x68
    volatile uint32_t GPFEN[2];     // 0x70 - 0x74
    volatile uint32_t GPPULLEN[2];  // 0x94 - 0x98
    volatile uint32_t GPRIS[2];     // 0xA8 - 0xAA
    volatile uint32_t GPFSYS[2];    // 0xB4 - 0xB8
} Gpio;

/*
 * UART Peripheral
 */
typedef struct {
    volatile uint32_t DR;      // 0x00 - Data Register
    volatile uint32_t RSRECR;  // 0x04 - RX/TX Errors
    volatile uint32_t FR;      // 0x18 - Flags
    volatile uint32_t ILPR;    // 0x20 - IrDA Low Power
    volatile uint32_t IBRD;    // 0x24 - Integer Baud Rate
    volatile uint32_t FBRD;    // 0x28 - Fractional Baud Rate
    volatile uint32_t LCRH;    // 0x2C - Line Control
    volatile uint32_t CR;      // 0x30 - Control
    volatile uint32_t IFLS;    // 0x34 - Interrupt FIFO Levels
    volatile uint32_t IMSC;    // 0x38 - Interrupt Mask Set/Clear
    volatile uint32_t RIS;     // 0x3C - Raw Interrupt Status
    volatile uint32_t MIS;     // 0x40 - Masked Interrupt Status
    volatile uint32_t ICR;     // 0x44 - Interrupt Clear
    volatile uint32_t DMACR;   // 0x48 - DMA Control
    volatile uint32_t ITCR;    // 0x80 - Test Control
    volatile uint32_t ITPC;    // 0x84 - Test Pulse
    volatile uint32_t ITOP;    // 0x88 - Test Operation
    volatile uint32_t TDER;    // 0x8C - Test Data
} Uart;

/*
 * Memory-mapped I/O helper functions
 */
static volatile void* map_peripheral(volatile void* base, uint32_t offset) {
    return (volatile void*)((uint8_t*)base + offset);
}

/*
 * UART Driver Implementation
 */
class UartDriver {
public:
    UartDriver() {
        // Initialize UART0 for 115200 baud
        // This is a simplified setup - real implementation would handle
        // proper UART configuration
    }

    void putc(uint8_t c) {
        // Map to UART0 DR register
        // Check FR register for TX FIFO not full
        while ( (((Uart*)UART0_BASE)->FR & 0x20) != 0 );  // Wait for TX FIFO not full
        (((Uart*)UART0_BASE)->DR) = c;  // Write character
    }
};

class GpioDriver {
public:
    GpioDriver() {
        // Setup GPIO
        // Typically you'd configure pin 14 (TX) and pin 15 (RX) for UART
        // and pin 21 for LED if needed
    }

    void set(uint8_t pin, uint8_t value) {
        if (value) {
            ((Gpio*)GPIO_BASE)->GPSET[pin / 32] = 1 << (pin % 32);
        } else {
            ((Gpio*)GPIO_BASE)->GPCLR[pin / 32] = 1 << (pin % 32);
        }
    }
};

class SystemDriver {
public:
    uint64_t get_time() {
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        return (uint64_t)ts.tv_sec * 1000000000 + ts.tv_nsec;
    }
};

// Global drivers
static UartDriver g_uart;
static GpioDriver g_gpio;
static SystemDriver g_system;

/*
 * FlintJVM Native Interface Implementations
 */

// Native method implementations for Java classes

jint Native::native_system_get_time() {
    return g_system.get_time();
}

jvoid Native::native_uart_putc(jbyte c) {
    g_uart.putc((uint8_t)c);
}

jvoid Native::native_pin_set(jint pin, jint value) {
    g_gpio.set((uint8_t)pin, (uint8_t)value);
}

// Additional native methods for common Java functions
jint Native::native_math_random() {
    // Simple pseudo-random number generator (not cryptographically secure)
    static uint32_t seed = 0x12345678;
    seed = seed * 1103515245 + 12345;
    return (jint)(seed & 0x7fffffff);
}

jboolean Native::native_system_exit(jint code) {
    // Exit the application
    // Use Flint's exit function
    Flint::exit(code);
    return true;
}

// Get system time in milliseconds (Java long)
jlong Native::native_system_current_time_millis() {
    return g_system.get_time() / 1000000;
}

// Sleep for a specified number of milliseconds
jvoid Native::native_system_nap(jint millis) {
    struct timespec ts;
    ts.tv_sec = millis / 1000;
    ts.tv_nsec = (millis % 1000) * 1000000;
    nanosleep(&ts, NULL);
}

// Get a random boolean
jboolean Native::native_boolean_random() {
    return (Native::native_math_random() % 2) != 0;
}

// Implement other common native functions based on FlintJVM's Native Interface
jbyte Native::native_byte_random() {
    return (jbyte)(Native::native_math_random() & 0xFF);
}

jchar Native::native_char_random() {
    return (jchar)(Native::native_math_random() & 0xFFFF);
}

jshort Native::native_short_random() {
    return (jshort)(Native::native_math_random() & 0xFFFF);
}

jlong Native::native_long_random() {
    uint32_t part1 = Native::native_math_random();
    uint32_t part2 = Native::native_math_random();
    return ((jlong)part1 << 32) | part2;
}

jfloat Native::native_float_random() {
    // Convert 32-bit random to float (simplified)
    union {
        float f;
        uint32_t i;
    } u;
    u.i = Native::native_math_random();
    return u.f;
}

jdouble Native::native_double_random() {
    // Convert 64-bit random to double
    union {
        double d;
        struct {
            uint32_t hi;
            uint32_t lo;
        } i;
    } u;
    u.i.hi = Native::native_math_random();
    u.i.lo = Native::native_math_random();
    return u.d;
}

// String handling functions
jstring Native::native_string_value_of(jint value) {
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "%d", value);
    return Flint::newAscii(buffer);
}

jstring Native::native_string_value_of(jlong value) {
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "%lld", value);
    return Flint::newAscii(buffer);
}

jstring Native::native_string_value_of(jfloat value) {
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "%f", value);
    return Flint::newAscii(buffer);
}

jstring Native::native_string_value_of(jdouble value) {
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "%lf", value);
    return Flint::newAscii(buffer);
}

jstring Native::native_string_value_of(jboolean value) {
    return Flint::newAscii(value ? "true" : "false");
}
