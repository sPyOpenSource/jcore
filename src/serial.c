#include "types.h"
#include "misc.h"
#include "config.h"

#ifdef QEMU
#define UART_BASE 0x09000000
#else
#define UART3_BASE 0x49020000
#endif

int debug_port = 0;

void init_serial(int port)
{
    (void)port;
#ifdef QEMU
    volatile unsigned int *cr = (volatile unsigned int *)(UART_BASE + 0x030);
    volatile unsigned int *lcr_h = (volatile unsigned int *)(UART_BASE + 0x02C);
    volatile unsigned int *ibrd = (volatile unsigned int *)(UART_BASE + 0x024);
    volatile unsigned int *fbrd = (volatile unsigned int *)(UART_BASE + 0x028);
    *cr = 0;
    *ibrd = 26;
    *fbrd = 3;
    *lcr_h = (3 << 5) | (1 << 4);
    *cr = (1 << 0) | (1 << 8) | (1 << 9);
#else
    volatile unsigned int *cr = (volatile unsigned int *)(UART3_BASE + 0x030);
    volatile unsigned int *lcr_h = (volatile unsigned int *)(UART3_BASE + 0x02C);
    volatile unsigned int *ibrd = (volatile unsigned int *)(UART3_BASE + 0x024);
    volatile unsigned int *fbrd = (volatile unsigned int *)(UART3_BASE + 0x028);
    *cr = 0;
    /* 115200 baud: 100MHz / (16 * 115200) = 54.25 -> IBRD=54, FBRD=4 */
    *ibrd = 54;
    *fbrd = 4;
    *lcr_h = (3 << 5) | (1 << 4); // 8 bits, FIFO enabled
    *cr = (1 << 0) | (1 << 8) | (1 << 9); // UART enable, TX enable, RX enable
#endif
}

static void ser_putchar_raw(int port, int ch)
{
    (void)port;
#ifdef QEMU
    volatile unsigned int *fr = (volatile unsigned int *)(UART_BASE + 0x018);
    volatile unsigned char *dr = (volatile unsigned char *)(UART_BASE + 0x000);
    while (*fr & (1 << 5));
    *dr = (unsigned char)ch;
#else
    volatile unsigned int *lsr = (volatile unsigned int *)(UART3_BASE + 0x14);
    volatile unsigned char *thr = (volatile unsigned char *)(UART3_BASE + 0x00);
    while (!(*lsr & (1 << 5)));
    *thr = (unsigned char)ch;
#endif
}

void ser_putchar(int port, int ch)
{
    if (ch == '\n') {
        ser_putchar_raw(port, '\r');
        ser_putchar_raw(port, '\n');
    } else {
        ser_putchar_raw(port, ch);
    }
}

void dbg_print(int port, char *s)
{
    while (*s) {
        ser_putchar(port, *s);
        s++;
    }
}

void putchar(int ch)
{
    ser_putchar(debug_port, ch);
}

void puts(char *s)
{
    dbg_print(debug_port, s);
}

unsigned char ser_waitforchar(int port)
{
    (void)port;
#ifdef QEMU
    volatile unsigned int *fr = (volatile unsigned int *)(UART_BASE + 0x018);
    volatile unsigned char *dr = (volatile unsigned char *)(UART_BASE + 0x000);
    for (;;) {
        if (!(*fr & (1 << 4)))
            return *dr;
    }
#else
    volatile unsigned int *lsr = (volatile unsigned int *)(UART3_BASE + 0x14);
    volatile unsigned char *rbr = (volatile unsigned char *)(UART3_BASE + 0x00);
    for (;;) {
        if (*lsr & 1)
            return *rbr;
    }
#endif
}

int ser_trygetchar(int port)
{
    (void)port;
#ifdef QEMU
    volatile unsigned int *fr = (volatile unsigned int *)(UART_BASE + 0x018);
    volatile unsigned char *dr = (volatile unsigned char *)(UART_BASE + 0x000);
    if (!(*fr & (1 << 4)))
        return *dr;
#else
    volatile unsigned int *lsr = (volatile unsigned int *)(UART3_BASE + 0x14);
    volatile unsigned char *rbr = (volatile unsigned char *)(UART3_BASE + 0x00);
    if (*lsr & 1)
        return *rbr;
#endif
    return -1;
}

void ser_dump(int port, const char *ptr, unsigned int size)
{
    while (size--) {
        ser_putchar_raw(port, *ptr++);
    }
}

void ser_putc(int x, char c)
{
    (void)x;
    ser_putchar(debug_port, c);
}
