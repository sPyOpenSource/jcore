
#include "kernel.h"
#include <stdint.h>
#include <string.h>
#include "flint.h"
#include "flint_system_api.h"

// PL011 UART registers (RPi3: ARM_IO_BASE + 0x201000)
#define UART0_BASE      0x3F201000
#define UART_DR         ((volatile uint32_t *)(UART0_BASE + 0x000))
#define UART_FR         ((volatile uint32_t *)(UART0_BASE + 0x018))
#define UART_IBRD       ((volatile uint32_t *)(UART0_BASE + 0x024))
#define UART_FBRD       ((volatile uint32_t *)(UART0_BASE + 0x028))
#define UART_LCRH       ((volatile uint32_t *)(UART0_BASE + 0x02C))
#define UART_CR         ((volatile uint32_t *)(UART0_BASE + 0x030))

#define FR_TXFF         (1 << 5)

static int uart_inited = 0;

static void uart_init(void) {
    volatile uint32_t *gpio = (volatile uint32_t *)0x3F200000;
    uint32_t sel;

    // GPIO 14, 15 -> ALT0 (PL011 TXD/RXD)
    sel = gpio[1];
    sel &= ~(0x7 << 12);
    sel |= (0x4 << 12);
    sel &= ~(0x7 << 15);
    sel |= (0x4 << 15);
    gpio[1] = sel;

    // Disable UART while configuring
    *UART_CR = 0;
    // 48 MHz clock / (16 * 115200) = 26.0417
    *UART_IBRD = 26;
    *UART_FBRD = 3;
    // 8N1, enable FIFO
    *UART_LCRH = (0x3 << 5) | (1 << 4);
    // Enable UART, TX, RX
    *UART_CR = (1 << 0) | (1 << 8) | (1 << 9);

    uart_inited = 1;
}

static void uart_putc(char c) {
    if (!uart_inited) uart_init();
    while (*UART_FR & FR_TXFF) {}
    *UART_DR = (unsigned char)c;
    if (c == '\n') {
        while (*UART_FR & FR_TXFF) {}
        *UART_DR = '\r';
    }
}

static void uart_puts(const char *s) {
    if (!uart_inited) uart_init();
    while (*s) {
        uart_putc(*s++);
    }
}

// Reserve 1MB for JVM heap
static uint8_t jvm_heap[0x100000] __attribute__((aligned(16)));

extern "C" {
    void rpi_uart_puts(const char *s) {
        uart_puts(s);
    }

    void rpi_uart_putc(char c) {
        uart_putc(c);
    }

    uint64_t rpi_get_ticks(void) {
        uint64_t ticks;
        asm volatile("mrs %0, cntvct_el0" : "=r"(ticks));
        return ticks;
    }
    bool Flint_runToMain(const char *cls);
}

CKernel::CKernel(void)
: m_ActLED()
{
}

CKernel::~CKernel(void)
{
}

boolean CKernel::Initialize(void)
{
    uart_puts("FlintJVM-RPi: Initializing...\r\n");
    return TRUE;
}

TShutdownMode CKernel::Run(void)
{
    uart_puts("FlintJVM-RPi: Starting JVM...\r\n");

    void *heapStart = jvm_heap;
    void *heapEnd = (void *)((uintptr_t)jvm_heap + sizeof(jvm_heap));
    FlintAPI::System::initHeap(heapStart, heapEnd);
    uart_puts("  heap init done\r\n");

    bool result = Flint_runToMain("com/rpi/HelloRPi");

    if (result) {
        uart_puts("FlintJVM-RPi: JVM execution completed.\r\n");
    } else {
        uart_puts("FlintJVM-RPi: JVM execution failed.\r\n");
    }

    return ShutdownHalt;
}
